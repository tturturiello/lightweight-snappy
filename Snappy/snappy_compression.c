#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"
#define MAX_BLOCK_SIZE 65536
#define MAX_HTABLE_SIZE 4096


#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


const int tab32[32] = {
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
void move_current(Buffer *bf, unsigned int offset){
    bf->current += offset;
    bf->bytes_left -= offset;
}

/**
 * Assegna la posizione di current a quella di beginning resettando cos?
 * il buffer
 * @param bf il buffer da resettare
 */
void reset_buffer(Buffer *bf) {
    bf->current = bf->beginning;
}


typedef struct compressor{
    unsigned short *hash_table;
    unsigned htable_size;
    u32 shift;
    u32 skip_bytes;
    u32 current_u32;
    int current_index;
    Node *copy; //TODO Che schifo?
} Compressor;

/*
void init_compressor_tree(Compressor *cmp){
    cmp->hash_table = (Tree **)malloc(sizeof(Tree*)*htable_size);
    for (int i = 0; i < htable_size; i++) {
        cmp->hash_table[i] = create_tree();
    }
}
*/

/**
 * Inizializza il Compressor passato in parametro allocando un'hash table di dimensione 4096.
 * N.B: anche se la memoria allocata ? una costante, la dimensione dell'hash table utilizzata
 * in compressione varia proporzionalmente alla dimensione del blocco in compressione
 * @param cmp il Compressor da inizializzare
 */
void init_compressor(Compressor *cmp){
    cmp->hash_table = (unsigned short *)calloc(MAX_HTABLE_SIZE, sizeof(unsigned short *));
}

static FILE *finput;
static FILE *fcompressed;
static unsigned long long finput_size;
static Buffer input;
static Buffer output;
static Compressor cmp;
static unsigned int literal_length;
//Data for testing
unsigned long long number_of_u32 = 0;
unsigned long long collisions = 0;
double time_taken = 0;

unsigned int find_copy_length(char *input, char *candidate, const char *limit) {//TODO: max copy length? Incremental copy?
    unsigned int length = 0;
    for(;input <= limit; input++, candidate++){
        if(*input==*candidate){
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

Tree **get_hash_table(int file_size) {
    Tree **hash_table = (Tree **)malloc(sizeof(Tree*)*10);
    return hash_table;
}

char *write_literal(const char *input, char *output, unsigned int len) {

    unsigned int len_minus_1 = len-1;
    if(len_minus_1 < 60) {
        *output++ = (len_minus_1 << 2u) & 0xFF;
    } else {
        char *tag_byte = output++; //Lascio lo spazio per il tag byte
        unsigned int code_literal = 59; //identifica quanti sono i byte utilizzati per codificare len-1

        while(len_minus_1 > 0){
            *output++ = len_minus_1 & 0xFF;
            len_minus_1 = len_minus_1 >> 8;
            code_literal++;
        }
        assert(code_literal >= 60);
        assert(code_literal <= 64);
        *tag_byte = code_literal << 2;
    }

    //printf("Literal di dimensione %d\n", len);
    for (int i = 0; i < len; ++i) {//TODO memcpy()?
        *output++ = input[i];
        //printf("[%d]: %X ",i, input[i]);
    }
    //printf("\n");
    return output;

}

char *write_single_copy(char *output, unsigned int len, unsigned int offset){
    if( (len < 12) && offset < 2048){//Copy 01: 3 bits for len-4 and 11 bits for offset
        *output++ = ((offset >> 8 ) << 5) + ((len - 4) << 2) + 1;
        *output++ = offset & 0xFF;
    } else if ( offset < 65536) {//Copy 10: 6 bits for len-1 and 16 bits for offset //TODO: assert?
        *output++ = ((len - 1)  << 2)  | 2;
        *output++ = offset & 0xFF;
        *output++ = (offset >> 8) & 0xFF;
        //Copy 11 non ? necessaria: il blocco da comprimere ? <= 64kB
    }
    return output;
}

char *write_copy(char *output, unsigned int len, unsigned long offset) {

    while(len > 68){ //Garantisco che rimangano un minimo di 4 bytes per utilizzare alla fine la copia 01
        output = write_single_copy(output, 64, offset); //64 ? la max len per una copia
        len-=64;
    }
    if(len > 64) { //64 < len < 68
        output = write_single_copy(output, 60, offset);
        len-=60;
    }

    output = write_single_copy(output, len, offset);
    return output;

}

/**
 * Scrive sul buffer di output la dimensione del file di input in formato varint.
 * Quest'informazione sar? poi utilizzata in decompressione
 */
void write_dim_varint() {
    unsigned int size_varint = parse_to_varint(finput_size, output.current);
    output.current += size_varint;
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
void init_buffers() {
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
static void load_next_block() {
    input.bytes_left = fread(input.current, sizeof(char), MAX_BLOCK_SIZE, finput);
    set_htable_size();
}

/**
 * Controlla se l'input buffer in compressione ? pieno o vuoto.
 * @return 1 se l'input buffer ? pieno, 0 altrimenti
 */
int input_is_full() {
    return input.bytes_left != 0;
}

/**
 * Controlla se la compressione ? vicina ad esaurire l'input buffer.
 * Nel dettaglio l'input viene considerato esaurito quando i byte rimanenti sono
 * meno di quelli che andrebbero consumati alla prossima iterazione
 * @return 1 se l'input ? quasi esaurito
 */
int is_block_end() {

    return input.bytes_left < (cmp.skip_bytes++ >> 5) + 15;//TODO, margine migliore?
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
    cmp.current_u32 = get_next_u32(input.current);
    cmp.current_index = hash_bytes(cmp.current_u32);
    number_of_u32++;//TODO togliere?
}

int found_match_tree() {

    if ( !is_empty(cmp.hash_table[cmp.current_index]) ) {
        return (cmp.copy = find(cmp.current_u32, cmp.hash_table[cmp.current_index])) != NULL;//Salvo anche il nodo copia
    }
    return 0;
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

void start_new_literal() {
    literal_length = 0;
    cmp.skip_bytes = 32;

}

void append_literal() {
    u32 bytes_to_skip = cmp.skip_bytes++ >> 5;
    literal_length += bytes_to_skip;
    move_current(&input, bytes_to_skip);
}

void exhaust_input() {

    literal_length += input.bytes_left;
    move_current(&input, input.bytes_left);

}

void update_hash_table_tree() {
    u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
    insert(previous_u32, input.current - input.beginning - 1, cmp.hash_table[hash_bytes(previous_u32)]);
    insert(cmp.current_u32, input.current - input.beginning , cmp.hash_table[cmp.current_index]);
}

static inline void update_hash_table() {
    u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
    cmp.hash_table[hash_bytes(previous_u32)] = input.current - input.beginning - 1;
    cmp.hash_table[hash_bytes(cmp.current_u32)] = input.current - input.beginning;
}

void emit_literal() {
    if(literal_length > 0)
        output.current = write_literal(input.current - literal_length, output.current, literal_length);
}

void emit_copy_tree() {
    char *candidate = input.beginning + cmp.copy->offset;
    int copy_length = 4 + find_copy_length(input.current + 4, candidate + 4, input.current + input.bytes_left);
    output.current = write_copy( output.current, copy_length, input.current - candidate);
    cmp.copy->offset = input.current - input.beginning; //Aggiorno l'offset della copia
    printf("%X copy of offset = %d and length = %d\n",cmp.current_u32, input.current - candidate, copy_length);

    move_current(&input, copy_length);
}

void emit_copy() {
    char *candidate = input.beginning + cmp.hash_table[cmp.current_index];
    int copy_length = 4 + find_copy_length(input.current + 4, candidate + 4, input.current + input.bytes_left);
    output.current = write_copy( output.current, copy_length, input.current - candidate);
    cmp.hash_table[cmp.current_index] = input.current - input.beginning; //Aggiorno l'offset della copia
    //printf("%X copy of offset = %d and length = %d\n",cmp.current_u32, input.current - candidate, copy_length);

    move_current(&input, copy_length);
}

void write_block_compressed() {
    fwrite(output.beginning, sizeof(char), output.current - output.beginning, fcompressed);
}

void reset_hash_table_tree() {
    for (int i = 0; i < MAX_HTABLE_SIZE; i++) {
        free_tree(cmp.hash_table[i]);
        cmp.hash_table[i] = create_tree();
    }
}

void reset_hash_table() {
    memset(cmp.hash_table, 0, MAX_HTABLE_SIZE * sizeof(unsigned short *));
}

void reset_buffers() {
    reset_buffer(&input);
    reset_buffer(&output);
}

void free_hash_table() {
    free(cmp.hash_table);
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


/*    puts("\n\nBuffer in input");
    for(int i = 0; beginning + i < input_limit; i++){
        printf("%3d: %X ", i, *(beginning+i));
    }

    puts("\n\nBuffer in output");
    for(int i = 0; out_beginning + i <= output; i++){
        printf("%X ", *(out_beginning+i));
    }*/

}

void compress_next_block() {

    start_new_literal(); //All'inizio di ogni blocco c'? sempre un literal
    append_literal();//Ignoro i primi 4 bytes

    while(!is_block_end()){

        generate_hash_index();

        if(found_match()){//TODO: Heuristic match skipping
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

void print_htable() {
    for (int i = 0; i < cmp.htable_size; ++i) {
        printf("%hu\t", cmp.hash_table[i]);
    }
    printf("\n\n\n-----------------------------------\n\n\n");
}

int snappy_compress(FILE *file_input, unsigned long long input_size, FILE *file_compressed) {

    clock_t t;
    t = clock();

    finput = file_input;
    fcompressed = file_compressed;
    finput_size = input_size;

    init_compressor(&cmp);
    init_buffers();//TODO passare environment in parametro

    write_dim_varint();
    load_next_block();

    while(input_is_full()){
        compress_next_block();

        write_block_compressed();

        //print_htable();//TODO Solo per test

        reset_hash_table();
        reset_buffers();
        load_next_block();
    }

    //TODO liberare memoria
    free_hash_table();

    t = clock() - t;
    time_taken = ((double)t)/CLOCKS_PER_SEC;

}


/*    int literal_length = 0;
    int copy_length = 0;
    const char *input_file_name = "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\wikipedia_test.txt";
    FILE *finput;
    char *input;
    char *output;
    const char * beginning;
    const char * out_beginning;
    char *input_limit;


    Tree *hash_table[htable_size];
    for (int i = 0; i < htable_size; i++) {
        hash_table[i] = create_tree();
    }

    if((finput = fopen(input_file_name, "r") )!= NULL){
        unsigned int file_size = getFileSize(input_file_name);
        input = (char *)calloc(file_size, sizeof(char));
        output = (char *)malloc(sizeof(char)*file_size);
        printf("LETTI:%d\n", file_size = fread(input, sizeof(char), file_size, finput) );
        printf("Dimesnione file: %d bytes, %d u32\n", file_size, file_size/4);
        beginning = input;
        out_beginning = output;
        input_limit = input + file_size;

        output = write_dim_varint(file_size, output);

    } else {
        puts("Errore apertura file");
        return 1;
    }
    fclose(finput);

    u32 current_u32 ;
    int index = 0;
    Node *copy;
    char *candidate;
    while( input+4 < input_limit ) {
        current_u32 = get_next_u32(input, input_limit);
        index = hash_bytes(current_u32);
        if (is_empty(hash_table[index]) | ((copy = find(current_u32, hash_table[index]) ) == NULL)) {
            //printf("%X literal\n", current_u32);
            //output[literal_length++] = current_u32; //Aggiungo il literal in output
            insert(current_u32, input - beginning , hash_table[index]);
            literal_length+=4;
            copy_length = 0;
        } else {

            output = write_literal(input - literal_length, output, literal_length);
            literal_length = 0;
            candidate = beginning + copy->offset;
            copy_length = find_copy_length(input+4, candidate + 4, input_limit);
            printf("%X copy of offset = %d and length = %d\n", current_u32, input - candidate, copy_length + 4);
            output = write_copy( output, copy_length + 4, input - candidate);
        }
        input+= 4 + copy_length;
    }
    output = write_literal(input - literal_length, output, literal_length);


    write_file_compressed(out_beginning, output);
    print_result_compression(finput, output, beginning, out_beginning, input_limit);*/
/*    puts("\n");
for (int i = 0; i < htable_size; ++i) {
    print_tree_inorder(hash_table[i]);
    printf("\n");
}*/

