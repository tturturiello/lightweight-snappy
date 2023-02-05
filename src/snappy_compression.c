#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "varint.h"
#include "BST.h"
#include "buffer_compression.h"

#define MAX_BLOCK_SIZE 65536
#define MAX_HTABLE_SIZE 4096

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
 * N.B: anche se la memoria allocata è una costante, la dimensione dell'hash table utilizzata
 * in compressione varia proporzionalmente alla dimensione del blocco in compressione
 * @param cmp il Compressor da inizializzare
 */
static void init_compressor(Compressor *cmp){
    cmp->hash_table = (unsigned short *)calloc(MAX_HTABLE_SIZE, sizeof(unsigned short *));
}

static FILE *finput;
static FILE *fcompressed;
static Buffer input;
static Buffer output;
static Compressor cmp;

/**
 *  Calcola la parte intera del logaritmo in base due dell'intero passato in parametro.
 *  Nel programma viene usata unicamente con potenze di due.
 * @param pow_of_2 la potenza di due
 * @return log2 del valore passato in parametro
 */
static inline int log2_32(unsigned int pow_of_2) {
    assert(pow_of_2 > 0);
    int pow = -1;
    while(pow_of_2 > 0){
        pow_of_2>>=1;
        pow++;
    }
    return pow;
}

/**
 * Calcola la lunghezza della copia trovata. Compara byte a byte le due sequenze in maniera lineare
 * fino a quando i due byte sono uguali o si abbia esaurito l'input.
 * @param current la posizione corrente nel buffer di input
 * @param candidate la possibile copia nella finestra di dati precedente
 * @return la lunghezza della copia
 */
static inline unsigned int find_copy_length(char * current, char *candidate) {
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

/**
 * Funzione di hash utilizzata da Snappy. Minimizza le collisioni e massimizza la velocità,
 * affidandosi a due sole operazioni: una moltiplicazione e uno shift. Il valore di shift è
 * proporzionale alla dimensione dell'hash table (32 - log2(htable_size)).
 * @param bytes i 4 byte di cui viene generato l'hash code
 * @return hash code dei 4 byte passati in parametro
 */
static inline u32 hash_bytes(u32 bytes){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) >> cmp.shift;
}

/**
 * Scrive in output il literal di lunghezza e inizio passati in parametro.
 * Il formato è quello specificato dal format description di Snappy. Se la lunghezza
 * del literal è inferiore a 60, len-1 viene scritto nei 6 higher bits del tag byte.
 * Se len > 60, len-1 viene scritto nei byte successivi al tag byte (da 1 a 4) e i 6 higher
 * bits del tag byte ne indicano la quantità: 60 -> 1 byte, 61 -> 2, 62 -> 3, 63 -> 4
 * @param start_of_literal l'inizio del literal
 * @param len la lunghezza del literal
 */
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
        assert(code_literal < 64);
        *tag_byte = code_literal << 2;//Inserisco il codice nel tag byte
    }

    memcpy(current_out, start_of_literal, len); //Copio il literal
    current_out += len;
    move_current(&output, current_out - output.current);

}

/**
 * Scrive una copia con len <= 64 (il massimo valore esprimibile in 6 bits) e offset specificato.
 * Il formato è quello specificato dal format description di Snappy. Vengono utilizzate solo copie 01 e 10,
 * dato che i blocchi in compressione non supera i 64kB e quindi un'offset non può eccedere questo valore.
 * Copy 01: 3 bits per len-4 (nel tag byte) e 11 bits per l'offset (3 ne l tag byte e 8 nel byte successivo)
 * Copy 10: 6 bits per len-1 (nel tag byte) e 2 byte successivi per l'ffset
 * @param len la lunghezza della copia (<=64)
 * @param offset l'offset della copia
 */
static inline void write_single_copy(unsigned int len, unsigned int offset){
    assert(len <= 64);
    char *current_out = output.current;
    if( (len < 12) && offset < 2048){//Copy 01: 3 bits for len-4 and 11 bits for offset
        *current_out++ = ((offset >> 8 ) << 5) + ((len - 4) << 2) + 1;
        *current_out++ = offset & 0xFF;
    } else if ( offset < 65536) {//Copy 10: 6 bits for len-1 and 16 bits for offset
        *current_out++ = ((len - 1) << 2) | 2;
        *current_out++ = offset & 0xFF;
        *current_out++ = (offset >> 8) & 0xFF;
        //Copy 11 non è necessaria: il blocco da comprimere è <= 64kB
    }
    move_current(&output, current_out - output.current);

}

/**
 *  Scrive una copia di qualsiasi dimensione scomponendola in più copie di lunghezza <= 64.
 *  Minimizza il numero di copie singole emesse.
 * @param len la lunghezza della copia
 * @param offset l'offset della copia
 */
static inline void write_copy(unsigned int len, unsigned long offset) {

    while(len > 68){ //Garantisco che rimangano un minimo di 4 bytes per utilizzare alla fine la copia 01
        write_single_copy(64, offset); //64 è la max len per una copia
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
 * Quest'informazione sarà poi utilizzata in decompressione
 */
static void write_dim_varint(unsigned long long input_size) {
    unsigned int size_varint = parse_to_varint(input_size, output.current);
    move_current(&output, size_varint);
}

/**
 * Inizializza i buffer necessari alla compressione.
 * Il buffer di input ha una grandezza fissa di 65536 byte.
 * Il buffer di output deve tenere conto del fatto che il risultato della compressione
 * può essere più grande dell'input. Il caso peggiore si ha quando si ha una sequenza di literal di 61 byte
 * seguito da una copia 10 di 3 byte. Ovvero si deve aggiungere un byte ogni 65 bytes. 65536 / 65 ~ 1010.
 * Inoltre va tenuto conto dello spazio occupato dal varint ad inizio file.
 * È facilmente verificabile che con 10 byte in formato varint è possibile rappresentare tutte
 * le dimensioni del file originale che un programma di compressione può ragionevolmnte aspettarsi
 *
 */
static void init_buffers() {

    init_Buffer(&input, MAX_BLOCK_SIZE);
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
        cmp.htable_size <<= 1; //è sempre una potenza di due
    }
    cmp.shift = 32 - log2_32(cmp.htable_size); //Shift usato dalla funzione di hash
}

/**
 * Legge il prossimo blocco dal file in input e chiama set_htable_size() per aggiornare
 * la dimensione dell'hash table. Il blocco letto avrà dimensione <= 65536.
 */
static inline void load_next_block() {
    input.bytes_left = fread(input.current, sizeof(char), MAX_BLOCK_SIZE, finput);
    set_htable_size();
}

/**
 * Controlla se l'input buffer in compressione è pieno o vuoto.
 * @return 1 se l'input buffer è pieno, 0 altrimenti
 */
static inline int input_is_full() {
    return input.bytes_left != 0;
}

/**
 * Controlla se la compressione è vicina ad esaurire l'input buffer.
 * Nel dettaglio l'input viene considerato esaurito quando i byte rimanenti sono
 * meno di quelli che andrebbero consumati alla prossima iterazione
 * @return 1 se l'input è quasi esaurito
 */
static inline int is_block_end() {

    return input.bytes_left < (cmp.skip_bytes >> 5) + 15;
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
 * Genera il codice hash dei 4 bytes correnti, che verrà poi utilizzato come indice
 * per accedere all'hash table. Le informazioni sono salvate nei rispettivi campi del Compressor
 */
static inline void generate_hash_index() {

    //printf("%X %X %X %X\n", (char)input.current[0],(char)input.current[1], (char)input.current[2], (char)input.current[3]);//TODO
    cmp.current_u32 = get_next_u32(input.current);
    cmp.current_index = hash_bytes(cmp.current_u32);
}

/**
 * Controlla se alla posizione indicata dall'offset ottenuto dall'hash table,
 * esista effettivamente una copia dei 4 byte correnti: Potrebbe infatti essere solo una collisione
 * @return 1 se una copia è stata trovata
 */
static inline int found_match() {

    char *candidate = input.beginning + cmp.hash_table[cmp.current_index]; //Beginning + offset
    u32 candidate_u32 = get_next_u32(candidate);

    return candidate_u32 == cmp.current_u32;
}

/**
 * Inizia un nuovo literal azzerandone il contatore della lunghezza.
 * La ricerca di un match torna a essere eseguita ad ogny byte (32 >> 5 = 1)
 */
static inline void start_new_literal() {
    cmp.literal_length = 0;
    cmp.skip_bytes = 32;
}

/**
 * Accumula i byte in input appena letti e per cui non è stata trovata una copia nel prossimo literal da
 * emettere. Per aumentare la velocità di compressione, se 32 u32 vengono esaminati senza trovare una copia,
 * il programma comincia a cercare una copia ogni due byte, se ulteriori 32 u32 non hanno match, cerca ogni tre
 * e così via. Questo permette di avere un grande vantaggio con input che presentano parti molto grandi di dati non
 * comprimibili, dato che il programma capisce in fretta che non ha senso cercare delle copie.
 */
static inline void append_literal() {
    u32 bytes_to_skip = cmp.skip_bytes++ >> 5;
    cmp.literal_length += bytes_to_skip;
    move_current(&input, bytes_to_skip);
}

/**
 * Esaurisce l'input accumulandone la parte rimanente nel prossimo literal da emettere
 */
static inline void exhaust_input() {

    cmp.literal_length += input.bytes_left;
    move_current(&input, input.bytes_left);

}

/**
 * Aggiorna l'hash table con l'offset dell'u32 (4 byte) corrente. Per aumentare la compressione a scapito di poche
 * operazioni in più, viene inserito nell'hash table anche l'offset dell'u32 precedente
 */
static inline void update_hash_table() {
    u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
    cmp.hash_table[hash_bytes(previous_u32)] = input.current - input.beginning - 1;
    cmp.hash_table[hash_bytes(cmp.current_u32)] = input.current - input.beginning;
}

/**
 * Se la lunghezza del literal accumulato al momento della chiamata di questa funzione è maggiore di zero,
 * viene chiamata write_literal per emettere i dati non compressi fino alla posizione corrente sul buffer in output
 */
static inline void emit_literal() {
    if(cmp.literal_length > 0)
        write_literal(input.current - cmp.literal_length, cmp.literal_length);
}

/**
 * Verifica se la lunghezza della copia sia maggiore di 4 byte e successivamente chima write_copy() per
 * scrivere le informazioni sul match nel buffer di output. Infine aggiorna l'offset nell'hash table con quello
 * della sequenza corrente
 */
static inline void emit_copy() {
    char *candidate = input.beginning + cmp.hash_table[cmp.current_index];
    int copy_length = 4 + find_copy_length(input.current + 4, candidate + 4);
    write_copy(copy_length, input.current - candidate);
    cmp.hash_table[cmp.current_index] = input.current - input.beginning; //Aggiorno l'offset della copia
    move_current(&input, copy_length);
}

/**
 * Copia il buffer di output su file compresso
 */
static inline void write_block_compressed() {
    fwrite(output.beginning, sizeof(char), output.current - output.beginning, fcompressed);
}

/**
 * Azzera tutti gli elementi dell'hash table. Quest'opearazione particolarmente costosa è una delle principali
 * ragioni per cui la dimensione dell'hash table viene calcolata in maniera proporzionale alla dimensione del blocco in input.
 */
static inline void reset_hash_table() {
    memset(cmp.hash_table, 0, MAX_HTABLE_SIZE * sizeof(unsigned short *));
}

/**
 * Resetta i buffer di input e output
 */
static inline void reset_buffers() {
    reset(&input);
    reset(&output);
}

/**
 * Libera la memoria allocata per l'hash table
 */
static void free_hash_table() {
    free(cmp.hash_table);
}

/**
 * Libera la memoria allocata per i buffer
 */
static void free_buffers() {
    free(input.beginning);
    free(output.beginning);
}

/**
 * Inizializza le variabili globali necessarie alla compressione
 * @param file_input il file da comprimere
 * @param file_compressed il file compresso
 */
static void init_environment(FILE *file_input, FILE *file_compressed) {
    finput = file_input;
    fcompressed = file_compressed;
    init_compressor(&cmp);
    init_buffers();
}

/**
 * Comprime un blocco di dimensione <= 64kb
 */
static inline void compress_next_block() {

    start_new_literal(); //All'inizio di ogni blocco c'è sempre un literal
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

/**
 * Comprime il file di input specificato. Entrambi i file di input e di output (file compresso)
 * devono essere passati in paramentro già aperti in lettura
 * e scrittura come file binari (rb per l'input e wb per l'output). Va inoltre fornita la dimensione
 * del file da comprimere.
 * @param file_input il file da comprimere
 * @param input_size la dimensione del file da comprimere
 * @param file_compressed  il file compresso
 */
void snappy_compress(FILE *file_input, unsigned long long input_size, FILE *file_compressed) {

    init_environment(file_input, file_compressed);
    write_dim_varint(input_size);
    load_next_block();
    while(input_is_full()){
        compress_next_block();
        write_block_compressed();
        reset_hash_table();
        reset_buffers();
        load_next_block();
    }
    free_hash_table();
    free_buffers();
}



