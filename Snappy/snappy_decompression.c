//
// Created by Timothy Turturiello on 30.11.19.
//

#ifndef __CLEAR__
#define __CLEAR__
//#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <zconf.h>
#include <time.h>
#include <assert.h>
#include "varint.h"
#include "snappy_decompression.h"

//#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Compressed_test/alice_compressed"
//#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Decompressed_test/alice_de.txt"

#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Standard_test/1000000b5.snp"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Standard_test/1000000b5dec"

//#define FINPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Compressed_test\\alice_compressed.snp"
//#define FOUTPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Decompressed_test\\alice_decompressed.txt"

#define BUFFER_DIM 131072 // caso peggiore
/*
 * rispetto requisiti
 * soluzione proposta
 * Converter (strutture)
 * procedure test (risultati)
*/

//// converte una sequenza di byte contenuti in un array, in un numero
typedef union convertion {
    char byte_arr[4];
    unsigned long value;
} Converter;

typedef struct buffer {
    char* array;
    unsigned long mark;
    unsigned long mark_copy;
} Buffer;

static unsigned long long dim_input;
static unsigned long long dim_output;
static float t;
static float mbps;


void test();

void init_buffer(Buffer *buffer, unsigned long buf_dim);

void init_converter(Converter *converter);

Buffer *buffer_constructor(unsigned long buf_dim);

int convert(Buffer *buffer, unsigned char num_bytes, Converter *converter);

int is_in_buffer(Buffer *buffer, unsigned int bytes_number);

void check_dim_buffer(Buffer *buf_src, unsigned int to_check, FILE *source);

int open_resources(FILE **file_in, FILE **file_out);

void close_resources(FILE *file_in, FILE *file_out);

void temp_r_OS(Buffer *buf_src);

void flush(Buffer *buf_out, FILE *infile, unsigned long dim);

void rflush(Buffer *buf_dest, FILE *destination);

void wflush(Buffer *buf_dest, Buffer *buf_src, FILE *destination, FILE *source);

unsigned long decompressor(FILE *source, Buffer *buf_dest, Buffer *buf_src);

char* buf_curr_elem(Buffer *buffer);

char* buf_next_elem(Buffer *buffer);

char* buf_rel_elem(Buffer *buffer);

float lose_data(FILE *destination, float desired_dim);

unsigned int do_literal(Buffer *buf_src, Buffer *buf_dest, FILE *source, Converter *converter);

void do_copy(Buffer *buf_src, Buffer *buf_dest, FILE *source, int extra_bytes, int len, Converter *converter);

void print_result_decompression(unsigned long fdecompressed_size, unsigned long fcompressed_size);

unsigned long long get_file_size(FILE *file);

void write_copy(Buffer *buffer, int len, unsigned int offset);

void write_literal(Buffer *destination, Buffer *source, unsigned int len);

#endif
int main()
{
    FILE *source;
    FILE *destination;
    if (open_resources(&source, &destination))
        return 1;

    unsigned long uncomp_dim = varint_to_dim(source);

    // TODO: generalizzare attraverso dei costruttori
    Buffer *buf_src = (Buffer *) buffer_constructor(BUFFER_DIM);
    Buffer *buf_dest = (Buffer *) buffer_constructor(uncomp_dim);

    // carico il buffer di 64kb del contenuto del file compresso
    fread(buf_src->array, sizeof(char), BUFFER_DIM, source);

    int count = 0;
    unsigned long readed = 0;
    //readed += do_literal(buf_src, buf_dest, source);
    do {
        count++;
        readed += decompressor(source, buf_dest, buf_src);
    } while (readed < uncomp_dim);

    // scrivo il contenuto del buffer principale nel file
    fwrite(buf_dest->array, sizeof(char), uncomp_dim, destination);

    print_result_decompression(get_file_size(destination), get_file_size(source));
    return 0;
}


int open_resources(FILE **file_in, FILE **file_out)
{
    if ((*file_in = fopen(FINPUT_NAME, "rb"))== NULL) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
    if ((*file_out = fopen(FOUTPUT_NAME, "wb")) == NULL ) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
    return 0;
}

void close_resources(FILE *file_in, FILE *file_out)
{
    fclose(file_in);
    fclose(file_out);
}

/**
 * Assegna il valore 0 ai campi del buffer
 * @param buffer
 */
void reset_buffer(Buffer *buffer)
{
    buffer->mark = 0;
    buffer->mark_copy = 0;
}

/**
 * Inizializza la struttura di conversione assegnando il valore 0 ai suoi campi
 * @param converter
 */
void init_converter(Converter *converter)
{
    converter->value=0;
}

/**
 * Inizializza la struttura Buffer assegnando il valore 0 ai suoi campi
 * alloca memoria sulla heap per un array che viene puntato dal puntatore char* array della struttura Buffer
 * @param buffer
 * @param buf_dim
 * @param container
 */
void init_buffer(Buffer *buffer, const unsigned long buf_dim)
{
    buffer->mark = 0;
    buffer->mark_copy = 0;
    buffer->array = (char *) malloc(sizeof(char)*(buf_dim));
}

/**
 * Alloca memoria sulla heap per la struttura Buffer e la inizializza
 * @param buf_dim
 * @return
 */
Buffer *buffer_constructor(unsigned long buf_dim)
{
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    init_buffer(buffer, buf_dim);
    return buffer;
}

void flush(Buffer *buf_out, FILE *infile, unsigned long dim)
{
    fread(buf_out->array, sizeof(char), dim, infile);
}

/**
 * Restituisce l'elemento corrente del buffer relativo all'indice, mark all'interno della struttra Buffer
 * @param buffer
 * @return
 */
char* buf_curr_elem(Buffer *buffer)
{
    return &(buffer->array[buffer->mark]);
}

/**
 * Restituisce l'elemento successivo del buffer relativo all'indice mark+1, all'interno della struttra Buffer
 * @param buffer
 * @return
 */
char* buf_next_elem(Buffer *buffer)
{
    buffer->mark++;
    return &(buffer->array[buffer->mark]);
}

/**
 * Restituisce l'elemento relativo a un offset del buffer con indice mark_copy, all'interno della struttra Buffer
 * @param buffer
 * @return
 */
char* buf_rel_elem(Buffer *buffer)
{
    return &(buffer->array[buffer->mark_copy]);
}

float lose_data(FILE *destination, float desired_dim)
{
    fseek(destination, 0, SEEK_END);
    if (desired_dim != (float)ftell(destination))
        return 1-(float)ftell(destination)/desired_dim;
    return 0;
}

int is_readable(Buffer *buffer, unsigned long n_bytes, unsigned int length)
{
    // se dalla posizione corrente si supera la dimensione del buffer leggendo i byte extra
    if (buffer->mark + n_bytes >= BUFFER_DIM || buffer->mark + length >= BUFFER_DIM)
        return 0;
    return 1;
}

/**
 * Controlla che i prossimi byte da leggere siano effettivamente contenuti nel buffer su cui si sta lavorando.
 * @param buffer
 * @param bytes_number
 * @return 1 se il buffer contiene le informazioni
 */
int is_in_buffer(Buffer *buffer, unsigned int bytes_number)
{
    // se dalla posizione corrente si supera la dimensione del buffer leggendo i byte extra
    if (buffer->mark + bytes_number >= BUFFER_DIM)
        return 0;
    return 1;
}

/**
 * Effettua una chiamata a sistema per leggere da file di input,
 * se il buffer su cui si sta lavorando risulta effetivamente pieno.
 * @param buf_src
 * @param to_check
 * @param source
 */
void check_dim_buffer(Buffer *buffer, unsigned int to_check, FILE *file)
{
    if (!is_in_buffer(buffer, to_check)) {
        fseek(file, -(BUFFER_DIM - buffer->mark), 1);
        fread(buffer->array, sizeof(char), BUFFER_DIM, file);
        buffer->mark = 0;
    }
}

inline int convert(Buffer *buffer, unsigned char num_bytes, Converter *converter)
{
    for (int j = 0; j < num_bytes; ++j) {
        (*converter).byte_arr[j] = *buf_next_elem(buffer);
    }
    return converter->value;
}

inline unsigned int do_literal(Buffer *buf_src, Buffer *buf_dest, FILE *source, Converter *converter)
{
    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int len = 0;

    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char curr_byte = *buf_curr_elem(buf_src); // corrente
    unsigned char notag_value = (mask_notag&(curr_byte)) >> 2;

    switch (notag_value) {
        case 63:
            extra_bytes++;
        case 62:
            extra_bytes++;
        case 61:
            extra_bytes++;
        case 60:
            extra_bytes++;
            check_dim_buffer(buf_src, extra_bytes, source);
            // converto i byte associati alla lunghezza del buffer in valore intero
            len = convert(buf_src, extra_bytes, converter) + 1;
            check_dim_buffer(buf_src, len, source);
            write_literal(buf_dest, buf_src, len);
            break;
        default: // <60 len = val+1, extra_bytes = 0
            len = notag_value + 1;
            check_dim_buffer(buf_src, len, source);
            write_literal(buf_dest, buf_src, len);
    }
    return len;
}

inline void write_literal(Buffer *destination, Buffer *source, unsigned int len) {
    source->mark++;
    for (unsigned int i = 0; i < len; ++i, destination->mark++, source->mark++) {
        *buf_curr_elem(destination) = *buf_curr_elem(source);
        // printf("%x, ", (unsigned char)*buf_curr_elem(buf_dest));
    }
    source->mark--;
}

/**
 * Trascrive la copia trovata sullo stesso buffer di destinazione.
 * effettua le chiamate necessarie a controllare che la lettura delle informazioni siano
 * contenute interamete nel buffer di sorgente.
 * @param buf_src
 * @param buf_dest
 * @param source
 * @param extra_bytes
 * @param len
 * @param converter
 * @return
 */
void do_copy(Buffer *buf_src, Buffer *buf_dest, FILE *source, int extra_bytes, int len, Converter *converter)
{
    unsigned int offset = 0;
    check_dim_buffer(buf_src, extra_bytes, source);
    check_dim_buffer(buf_src, len, source);
    buf_dest->mark_copy = buf_dest->mark;

    // leggo i byte extra
    offset = convert(buf_src, extra_bytes, converter);
    // check offset
    if (buf_src->mark - offset < 0 || offset > BUFFER_DIM) {
        return;
    }

    write_copy(buf_dest, len, offset);
}

inline void write_copy(Buffer *buffer, int len, unsigned int offset) {
    // leggo a partire dall'offset
    buffer->mark_copy -= offset;
    // copio elemento per elemento
    for (unsigned int i = 0; i < len; ++i, buffer->mark++, buffer->mark_copy++) {
        *buf_curr_elem(buffer) = *buf_rel_elem(buffer);
    }
}

/**
 * Decomprime le informazioni contenute nel buffer di sorgente buf_src e le scrive nel buffer di destinazione buf_dest.
 * Effettua chiamate di controllo sui buffer.
 * @param source
 * @param buf_dest
 * @param buf_src
 * @return
 */
unsigned long inline decompressor(FILE *source, Buffer *buf_dest, Buffer *buf_src)
{
    static unsigned long counter;
    //unsigned char curr_byte = (buf_src->array[buf_src->mark]); // corrente
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // 00011100
    unsigned char mask_sx_notag = 0xE0; // 11100000
    unsigned char curr_byte = *buf_curr_elem(buf_src); // corrente
    unsigned char notag_value = (mask_notag&(curr_byte)) >> 2;

    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int len = 0;
    Converter converter;
    init_converter(&converter);

    unsigned char mode = mask_tag & curr_byte;
    switch(mode) {
        case 0:
            len = do_literal(buf_src, buf_dest, source, &converter);
            break;
        case 1: // copy 01
            extra_bytes = 1; // di lettura offset
            len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)
            // bit piu' significativi nel byte di tag
            converter.byte_arr[1] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111
            do_copy(buf_src, buf_dest, source, extra_bytes, len, &converter);
            break;
        case 2: // copy 10
            extra_bytes = 2;
            len = notag_value+1;
            do_copy(buf_src, buf_dest, source, extra_bytes, len, &converter);
            break;
        case 3: // copy 11
            extra_bytes = 4;
            len = notag_value+1;
            do_copy(buf_src, buf_dest, source, extra_bytes, len, &converter);
            break;
        default:break;
    }
    buf_src->mark++;
    counter++;
    return len;
}


unsigned long long get_file_size(FILE *file) {
    unsigned long long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

/**
 * Ospita la struttura principale dell'algoritmo di decompressione.
 * Facendo uso di due buffer applica la funzione decompressor fintanto che
 * il buffer di destinazione non raggiunge la dimensione del file non compresso,
 * assunta nei primi byte del file compresso ed espressa in varint.
 * @param file_input
 * @param file_decompressed
 * @return
 */
int snappy_decompress(FILE *file_input, FILE *file_decompressed)
{
    unsigned long uncomp_dim = varint_to_dim(file_input);
    Buffer *buf_src = (Buffer *) buffer_constructor(BUFFER_DIM);
    Buffer *buf_dest = (Buffer *) buffer_constructor(uncomp_dim);

    fread(buf_src->array, sizeof(char), BUFFER_DIM, file_input);
    unsigned long readed = 0;
    do {
        readed += decompressor(file_input, buf_dest, buf_src);

    } while (readed < uncomp_dim);

    // scrivo il contenuto del buffer principale nel file
    fwrite(buf_dest->array, sizeof(char), uncomp_dim, file_decompressed);
    free(buf_src);
    free(buf_dest);
    return 0;
}

void print_result_decompression(unsigned long fdecompressed_size, unsigned long fcompressed_size)
{
    printf("\nDimensione file compresso = %llu bytes\n", fcompressed_size);

    printf("Dimensione file decompresso = %llu bytes\n", fdecompressed_size);

    // double comp_ratio = (double)fcompressed_size / (double)finput_size;
    // printf("Compression ratio = %f\n", (double)finput_size / (double)fcompressed_size );
    // printf("Saving %f%%\n", (1 - comp_ratio)*100 );

    //printf("\nDecompression took %f seconds to execute\n", time_taken);
    // printf("%f MB/s\n", fcompressed_size/(time_taken * 1e6));
}

void write_result_decompression(unsigned long long fdecompressed_size)
{
    FILE *csv = fopen("..\\Standard_test\\risultati_decompressione.csv", "a");
    assert(csv!=NULL);
    fprintf(csv, "%llu, %llu, %f, %f\n",
            dim_input,
            dim_output,
            t,
            mbps);
    fclose(csv);
}