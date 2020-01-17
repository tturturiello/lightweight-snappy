#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "varint.h"
#include "BST.h"
#include "buffer_compression.h"
#define MAX_BLOCK_SIZE 65536
#define MAX_HTABLE_SIZE 4096

/**
 * Le funzioni senza documentazione sono copie dal file snappy_compression.c
 */

typedef struct compressor_bst{
    Tree **hash_table;
    unsigned htable_size;
    u32 shift;
    u32 skip_bytes;
    u32 current_u32;
    int current_index;
    unsigned int literal_length;
    Node *copy;
} Compressor_bst;

/**
 * Inizializza un Compressore con hash table di BST
 * Viene allocato un albero binario per ognni posizione dell'array
 * @param cmp
 */
void init_compressor_bst(Compressor_bst *cmp){
    cmp->hash_table = (Tree **)malloc(sizeof(Tree*)*MAX_HTABLE_SIZE);
    for (int i = 0; i < MAX_HTABLE_SIZE; i++) {
        cmp->hash_table[i] = create_tree();
    }
}


static FILE *finput;
static FILE *fcompressed;
static Buffer input;
static Buffer output;
static Compressor_bst cmp;

static inline int log2_32(unsigned int pow_of_2) {
    assert(pow_of_2 > 0);
    int pow = -1;
    while(pow_of_2 > 0){
        pow_of_2>>=1;
        pow++;
    }
    return pow;
}

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
    memcpy(current_out, start_of_literal, len); //Copio il literal
    current_out += len;
    move_current(&output, current_out - output.current);

}

static inline void write_single_copy(unsigned int len, unsigned int offset){
    assert(len <= 64);
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
        write_single_copy(64, offset); //64 è la max len per una copia
        len-=64;
    }
    if(len > 64) { //64 < len < 68
        write_single_copy(60, offset);
        len-=60;
    }

    write_single_copy(len, offset);
}

static void write_dim_varint(unsigned long long input_size) {
    unsigned int size_varint = parse_to_varint(input_size, output.current);
    move_current(&output, size_varint);
}

static void init_buffers() {
    init_Buffer(&input, MAX_BLOCK_SIZE); //TODO min?
    init_Buffer(&output, MAX_BLOCK_SIZE + 1010 + 5);
}

static inline void set_htable_size() {
    cmp.htable_size = 256; //Minimo
    while(cmp.htable_size < MAX_HTABLE_SIZE & cmp.htable_size < input.bytes_left){
        cmp.htable_size <<= 1; //? sempre una potenza di due
    }
    cmp.shift = 32 - log2_32(cmp.htable_size); //Shift usato dalla funzione di hash
}

static inline void load_next_block() {
    input.bytes_left = fread(input.current, sizeof(char), MAX_BLOCK_SIZE, finput);
    set_htable_size();
}

static inline int input_is_full() {
    return input.bytes_left != 0;
}

static inline int is_block_end() {

    return input.bytes_left < (cmp.skip_bytes++ >> 5) + 15;
}

static inline u32 get_next_u32(const unsigned char *input) {
    return (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3];
}

static inline void generate_hash_index() {
    cmp.current_u32 = get_next_u32(input.current);
    cmp.current_index =  hash_bytes(cmp.current_u32);
}

/**
 * Controlla se esiste una copia di 4 byte della sequenza corrente effettuando una ricerca binaria
 * nell'albero presente alla posizione associata nell'hash table. Per evitare di dover effettuare nuovamente la ricerca,
 * il risultato viene salvato nel campo copy di cmp (Compressor_bst)
 * @return 1 se un match è stato trovato
 */
static inline int found_match_tree() {

    if ( !is_empty(cmp.hash_table[cmp.current_index]) ) {
        return (cmp.copy = find(cmp.current_u32, cmp.hash_table[cmp.current_index])) != NULL;//Salvo anche il nodo copia
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

/**
 * Aggiorna l'hash table inserendo nell'albero binario la sequenza corrente e la sequenza -1
 */
static inline void update_hash_table_tree() {
    u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
    insert(previous_u32, input.current - input.beginning - 1, cmp.hash_table[hash_bytes(previous_u32)]);
    insert(cmp.current_u32, input.current - input.beginning , cmp.hash_table[cmp.current_index]);
}


static inline void emit_literal() {
    if(cmp.literal_length > 0)
        write_literal(input.current - cmp.literal_length, cmp.literal_length);
}

static inline void emit_copy_tree() {
    char *candidate = input.beginning + cmp.copy->offset;

    int copy_length = 4 + find_copy_length(input.current + 4,candidate + 4);
    write_copy(copy_length, input.current - candidate);
    cmp.copy->offset = input.current - input.beginning; //Aggiorno l'offset della copia
    move_current(&input, copy_length);
}


static inline void write_block_compressed() {
    fwrite(output.beginning, sizeof(char), output.current - output.beginning, fcompressed);
}

/**
 * Resetta l'hash table. Per ogni posizione viene liberata la memoria occupata dall'albero
 * costruito in precedenza e ne viene creato uno nuovo
 */
static inline void reset_hash_table_bst() {
    for (int i = 0; i < cmp.htable_size; i++) {
        free_tree(cmp.hash_table[i]);
        cmp.hash_table[i] = create_tree();
    }
}

static inline void reset_buffers() {
    reset(&input);
    reset(&output);
}

/**
 * Libera la memoria occupata dall'hash table. Dapprima libera ogni albero binario e successivamente
 * l'array di puntatori ad alberi binari vero e proprio.
 */
static void free_hash_table_bst() {
    for (int i = 0; i < cmp.htable_size; i++) {
        free_tree(cmp.hash_table[i]);
    }
    free(cmp.hash_table);
}

static void free_buffers() {
    free(input.beginning);
    free(output.beginning);
}

static void init_environment(FILE *file_input, FILE *file_compressed) {
    finput = file_input;
    fcompressed = file_compressed;
    init_compressor_bst(&cmp);
    init_buffers();
}

static inline void compress_next_block() {

    start_new_literal(); //All'inizio di ogni blocco c'è sempre un literal
    append_literal();

    while(!is_block_end()){

        generate_hash_index();
        if(found_match_tree()){
            emit_literal();
            start_new_literal();
            emit_copy_tree();
        } else {
            update_hash_table_tree();
            append_literal();
        }
    }
    exhaust_input();
    emit_literal();
}


int snappy_compress_bst(FILE *file_input, unsigned long long input_size, FILE *file_compressed) {

    init_environment(file_input, file_compressed);
    write_dim_varint(input_size);
    load_next_block();

    while(input_is_full()){
        compress_next_block();
        write_block_compressed();
        reset_hash_table_bst();
        reset_buffers();
        load_next_block();
    }
    free_hash_table_bst();
    free_buffers();
}



