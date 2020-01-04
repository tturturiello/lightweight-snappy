#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"

#define MAX_BLOCK_SIZE 65536
#define MAX_HTABLE_SIZE 4096
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


static const int tab32[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31};

static inline int log2_32(unsigned int value) {
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(unsigned int) (value * 0x07C4ACDD) >> 27];
}

typedef struct buffer {
    char *current;
    char *beginning;
    unsigned int bytes_left;
} Buffer;

/**
 * Inizializza il buffer allocando dinamicamente un array di byte di
 * dimensione buffer_size.
 * @param bf il buffer da inizializzare
 * @param buffer_size la dimensione del buffer
 */
static void init_Buffer(Buffer *bf, unsigned int buffer_size){
    bf->current = (char *)calloc(buffer_size, sizeof(char));
    bf->beginning = bf->current;
    bf->bytes_left = buffer_size;
}

/**
 * Sposta la posizione del puntatore current dell'offset specificato
 * nel buffer bf
 * @param bf il buffer da modificare
 * @param offset
 */
static inline void move_current(Buffer *bf, unsigned int offset){
    bf->current += offset;
    bf->bytes_left -= offset;
}

/**
 * Assegna la posizione di current a quella di beginning resettando cos?
 * il buffer
 * @param bf il buffer da resettare
 */
static void reset_buffer(Buffer *bf) {
    bf->current = bf->beginning;
}

typedef struct compressor{
    unsigned short *hash_table;
    unsigned htable_size;
    u32 shift;
    u32 skip_bytes;
    u32 current_u32;
    int current_index;
    unsigned int literal_length;
} Compressor;


/**
 * Inizializza il Compressor passato in parametro allocando un'hash table di dimensione 4096.
 * N.B: anche se la memoria allocata ? una costante, la dimensione dell'hash table utilizzata
 * in compressione varia proporzionalmente alla dimensione del blocco in compressione
 * @param cmp il Compressor da inizializzare
 */
static void init_compressor(Compressor *cmp){
    cmp->hash_table = (unsigned short *)calloc(MAX_HTABLE_SIZE, sizeof(unsigned short *));
}



static FILE *finput;
static FILE *fcompressed;
static unsigned long long finput_size;
static Buffer input;
static Buffer output;
static Compressor cmp;
//Data for testing
unsigned long long number_of_u32 = 0;
unsigned long long collisions = 0;
double time_taken = 0;


static inline unsigned int find_copy_length(char * current, char *candidate) {//TODO: max copy length? Incremental copy?
    const char *limit =  input.current + input.bytes_left;
    unsigned int length = 0;
    for(; current < limit; current++, candidate++){
        if(*current == *candidate){
            length++;
        } else {
            return length;
        }
    }
    return length;
}

static inline u32 hash_bytes(u32 bytes){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) >> cmp.shift;
}

static inline void write_literal(const char *start_of_literal, unsigned int len) {

    char *current_out = output.current;
    unsigned int len_minus_1 = len-1;
    if(len_minus_1 < 60) {
        *current_out++ = (len_minus_1 << 2u) & 0xFF;
    } else {
        char *tag_byte = current_out++; //Lascio lo spazio per il tag byte
        unsigned int code_literal = 59; //identifica quanti sono i byte utilizzati per codificare len-1

        while(len_minus_1 > 0){
            *current_out++ = len_minus_1 & 0xFF;
            len_minus_1 = len_minus_1 >> 8;
            code_literal++;
        }
        assert(code_literal >= 60);
        assert(code_literal <= 64);
        *tag_byte = code_literal << 2;
    }
/*    printf("Literal of len = %u\n", len);
    for (int i = 0; i < len; ++i) {
        printf("%X ", start_of_literal[i]);
    }
    puts("");*///TODO
    memcpy(current_out, start_of_literal, len); //Copio il literal
    current_out += len;
    move_current(&output, current_out - output.current);

}

static inline void write_single_copy(unsigned int len, unsigned int offset){
    char *current_out = output.current;
    if( (len < 12) && offset < 2048){//Copy 01: 3 bits for len-4 and 11 bits for offset
        *current_out++ = ((offset >> 8 ) << 5) + ((len - 4) << 2) + 1;
        *current_out++ = offset & 0xFF;
    } else if ( offset < 65536) {//Copy 10: 6 bits for len-1 and 16 bits for offset //TODO: assert?
        *current_out++ = ((len - 1) << 2) | 2;
        *current_out++ = offset & 0xFF;
        *current_out++ = (offset >> 8) & 0xFF;
        //Copy 11 non ? necessaria: il blocco da comprimere ? <= 64kB
    }
    move_current(&output, current_out - output.current);
}

static inline void write_copy(unsigned int len, unsigned long offset) {

    while(len > 68){ //Garantisco che rimangano un minimo di 4 bytes per utilizzare alla fine la copia 01
        write_single_copy(64, offset); //64 ? la max len per una copia
        len-=64;
    }
    if(len > 64) { //64 < len < 68
        write_single_copy(60, offset);
        len-=60;
    }

    write_single_copy(len, offset);
}

/**
 * Scrive sul buffer di output la dimensione del file di input in formato varint.
 * Quest'informazione sar? poi utilizzata in decompressione
 */
static void write_dim_varint() {
    unsigned int size_varint = parse_to_varint(finput_size, output.current);
    move_current(&output, size_varint);
}

/**
 * Inizializza i buffer necessari alla compressione.
 * Il buffer di input ha una grandezza fissa di 65536 byte.
 * Il buffer di output deve tenere conto del fatto che il risultato della compressione
 * pu? essere pi? grande dell'input. Il caso peggiore si ha quando si ha una sequenza di literal di 61 byte
 * seguito da una copia 10 di 3 byte. Ovvero si deve aggiungere un byte ogni 65 bytes. 65536 / 65 ~ 1010.
 * Infine va tenuto conto dello spazio occupato dal varint nell'output buffer del primo blocco: la dimensione
 * massima del file in compressione ? 4 Gb, che occupa 5 bytes in formato varint.
 *
 */
static void init_buffers() {
    init_Buffer(&input, MAX_BLOCK_SIZE); //TODO min?
    init_Buffer(&output, MAX_BLOCK_SIZE + 1010 + 5);
}

/**
 * Questa funzione viene chiamata prima che la compressione del blocco abbia inizio.
 * La dimensione dell'htable viene settata proporzionalmente alla dimensione del blocco
 * da comprimere.
 */
static inline void set_htable_size() {
    cmp.htable_size = 256; //Minimo
    while(cmp.htable_size < MAX_HTABLE_SIZE & cmp.htable_size < input.bytes_left){
        cmp.htable_size <<= 1; //? sempre una potenza di due
    }
    cmp.shift = 32 - log2_32(cmp.htable_size); //Shift usato dalla funzione di hash

}

/**
 * Legge il prossimo blocco dal file in input e chiama set_htable_size() per aggiornare
 * la dimensione dell'hash table. Il blocco letto avr? dimensione <= 65536.
 */
static inline void load_next_block() {
    input.bytes_left = fread(input.current, sizeof(char), MAX_BLOCK_SIZE, finput);
    set_htable_size();
}

/**
 * Controlla se l'input buffer in compressione ? pieno o vuoto.
 * @return 1 se l'input buffer ? pieno, 0 altrimenti
 */
static inline int input_is_full() {
    return input.bytes_left != 0;
}

/**
 * Controlla se la compressione ? vicina ad esaurire l'input buffer.
 * Nel dettaglio l'input viene considerato esaurito quando i byte rimanenti sono
 * meno di quelli che andrebbero consumati alla prossima iterazione
 * @return 1 se l'input ? quasi esaurito
 */
static inline int is_block_end() {

    return input.bytes_left < (cmp.skip_bytes >> 5) + 15;//TODO, margine migliore?
}

/**
 * Converte i 4 bytes contigui a input in u32 (unsigned int)
 * @param input la posizione da cui caricare i 4 bytes
 * @return u32
 */
static inline u32 get_next_u32(const unsigned char *input) {
    return (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3];
}

/**
 * Genera il codice hash dei 4 bytes correnti, che verr? poi utilizzato come indice
 * per accedere all'hash table. Le informazioni sono salvate nei rispettivi campi del Compressor
 */
static inline void generate_hash_index() {

    //printf("%X %X %X %X\n", (char)input.current[0],(char)input.current[1], (char)input.current[2], (char)input.current[3]);//TODO
    cmp.current_u32 = get_next_u32(input.current);
    cmp.current_index = hash_bytes(cmp.current_u32);
    number_of_u32++;//TODO togliere?
}

static inline int found_match() {

    char *candidate = input.beginning + cmp.hash_table[cmp.current_index]; //Beginning + offset
    u32 candidate_u32 = get_next_u32(candidate);

    if(candidate_u32 == cmp.current_u32){
        return 1;
    } else if( cmp.hash_table[cmp.current_index] != 0 ){
        collisions++;
    }
    return 0;

}

static inline void start_new_literal() {
    cmp.literal_length = 0;
    cmp.skip_bytes = 32;

}

static inline void append_literal() {
    u32 bytes_to_skip = cmp.skip_bytes++ >> 5;
    cmp.literal_length += bytes_to_skip;
    move_current(&input, bytes_to_skip);
}

static inline void exhaust_input() {

    cmp.literal_length += input.bytes_left;
    move_current(&input, input.bytes_left);

}

static inline void update_hash_table() {
    u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
    cmp.hash_table[hash_bytes(previous_u32)] = input.current - input.beginning - 1;
    cmp.hash_table[hash_bytes(cmp.current_u32)] = input.current - input.beginning;
}

static inline void emit_literal() {
    if(cmp.literal_length > 0)
        write_literal(input.current - cmp.literal_length, cmp.literal_length);
}

static inline void emit_copy() {
    char *candidate = input.beginning + cmp.hash_table[cmp.current_index];
    int copy_length = 4 + find_copy_length(input.current + 4, candidate + 4);
    write_copy(copy_length, input.current - candidate);
    cmp.hash_table[cmp.current_index] = input.current - input.beginning; //Aggiorno l'offset della copia
    //printf("%X copy of offset = %d and length = %d\n",cmp.current_u32, input.current - candidate, copy_length);

    move_current(&input, copy_length);
}

static inline void write_block_compressed() {
    fwrite(output.beginning, sizeof(char), output.current - output.beginning, fcompressed);
}

static inline void reset_hash_table() {
    memset(cmp.hash_table, 0, MAX_HTABLE_SIZE * sizeof(unsigned short *));
}

static inline void reset_buffers() {
    reset_buffer(&input);
    reset_buffer(&output);
}

static void free_hash_table() {
    free(cmp.hash_table);
}

static void free_buffers() {
    free(input.beginning);
    free(output.beginning);
}

static void init_environment(FILE *file_input, unsigned long long int input_size, FILE *file_compressed) {
    finput = file_input;
    fcompressed = file_compressed;
    finput_size = input_size;
    init_compressor(&cmp);
    init_buffers();
}

void print_htable() {
    for (int i = 0; i < cmp.htable_size; ++i) {
        printf("%hu\t", cmp.hash_table[i]);
    }
    printf("\n\n\n-----------------------------------\n\n\n");
}

static inline void compress_next_block() {

    start_new_literal(); //All'inizio di ogni blocco c'? sempre un literal
    append_literal();

    while(!is_block_end()){

        generate_hash_index();
        if(found_match()){
            emit_literal();
            start_new_literal();
            emit_copy();
        } else {
            update_hash_table();
            append_literal();
        }
    }
    exhaust_input();
    emit_literal();
}


int snappy_compress(FILE *file_input, unsigned long long input_size, FILE *file_compressed) {

    //clock_t t;
    //t = clock();

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    init_environment(file_input, input_size, file_compressed);
    write_dim_varint();
    load_next_block();

/*    for (int i = 0; i < input.bytes_left; ++i) {
        printf("%d:%X\t",i, input.beginning[i]);
    }
    puts("");*///TODO
    while(input_is_full()){
        compress_next_block();
        write_block_compressed();
        reset_hash_table();
        reset_buffers();
        load_next_block();
    }
    free_hash_table();
    free_buffers();

    //t = clock() - t;
    //time_taken =((double)t)/CLOCKS_PER_SEC;
    QueryPerformanceCounter(&end);
    time_taken = (double) (end.QuadPart - start.QuadPart) / frequency.QuadPart;

}

void print_result_compression(unsigned long long fcompressed_size) {

    //TODO check se un compressione ? avvenuta

    printf("\nDimensione file originale = %llu bytes\n", finput_size);

    printf("Dimensione file compresso = %llu bytes\n", fcompressed_size);

    double comp_ratio = (double)fcompressed_size / (double)finput_size;
    printf("Compression ratio = %f\n", (double)finput_size / (double)fcompressed_size );
    printf("Saving %f%%\n", (1 - comp_ratio)*100 );

    printf("\nNumero di u32 processati = %llu\n", number_of_u32 );
    printf("Numero di collisioni = %llu\n", collisions );
    printf("In percentuale: %f%%\n", ((double)collisions / (double)number_of_u32)*100 );

    printf("\nCompression took %f seconds to execute\n", time_taken);
    printf("%f MB/s\n", finput_size/(time_taken * 1e6));
}

void write_result_compression(unsigned long long fcompressed_size){
    FILE *csv = fopen("..\\Standard_test\\risultati_compressione.csv", "a");
    assert(csv!=NULL);
    fprintf(csv, "%llu, %llu, %f, %f, %f\n", finput_size,
            fcompressed_size,
            (double)finput_size / (double)fcompressed_size ,
            time_taken,
            finput_size/(time_taken * 1e6));
    fclose(csv);
}

