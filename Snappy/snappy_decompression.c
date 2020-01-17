//
// Created by Timothy Turturiello on 30.11.19.
//

#ifndef __CLEAR__
#define __CLEAR__

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <stdio.h>
#include <stdlib.h>
#include "varint.h"
#include "snappy_decompression.h"

#define BUFFER_DIM 131072
#define CONVERTER_DIM 4

typedef union convertion {
    char byte_arr[CONVERTER_DIM];
    unsigned long value;
} Converter;

typedef struct buffer {
    char* array;
    unsigned long mark;
    unsigned long mark_copy;
} Buffer;

void init_buffer(Buffer *buffer, unsigned long buf_dim);

void init_converter(Converter *converter);

Buffer *buffer_constructor(unsigned long buf_dim);

int convert(Buffer *buffer, unsigned char num_bytes, Converter *converter);

int is_in_buffer(Buffer *buffer, unsigned int bytes_number);

void check_dim_buffer(Buffer *buffer, unsigned int to_check, FILE *file);

unsigned long decompressor(FILE *source, Buffer *buf_dest, Buffer *buf_src);

char* buf_curr_elem(Buffer *buffer);

char* buf_next_elem(Buffer *buffer);

char* buf_rel_elem(Buffer *buffer);

unsigned int do_literal(Buffer *buf_src, Buffer *buf_dest, FILE *source, Converter *converter);

void do_copy(Buffer *buf_src, Buffer *buf_dest, FILE *source, int extra_bytes, int len, Converter *converter);

void write_copy(Buffer *buffer, int len, unsigned int offset);

void write_literal(Buffer *destination, Buffer *source, unsigned int len);
#endif

/**
 * Azzera i campi della struttura buffer.
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
 * Inizializza la struttura buffer con valori pari a 0,
 * alloca memoria sullo heap per un array che viene puntato dalla struttura Buffer
 * @param buffer
 * @param buf_dim
 * @param container
 */
void init_buffer(Buffer *buffer, const unsigned long buf_dim)
{
    reset_buffer(buffer);
    buffer->array = (char *) malloc(sizeof(char)*(buf_dim));
}

/**
 * Alloca memoria sullo heap per la struttura Buffer e la inizializza
 * @param buf_dim
 * @return
 */
Buffer *buffer_constructor(unsigned long buf_dim)
{
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    init_buffer(buffer, buf_dim);
    return buffer;
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

/**
 * Controlla che i prossimi byte da leggere o scrivere possano essere contenuti interamente nel buffer passato come parametro.
 * Se dalla posizione corrente si supera la dimensione del buffer, la funzione resituisce 0.
 * @param buffer
 * @param bytes_number
 * @return 1 se il buffer contiene le informazioni
 */
int is_in_buffer(Buffer *buffer, unsigned int bytes_number)
{
    if (buffer->mark + bytes_number + 1 >= BUFFER_DIM)
        return 0;
    return 1;
}

/**
 * Effettua una chiamata a sistema per leggere da file di input e aggiornare il contenuto del buffer,
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

/**
 * Converte i valori contenuti nei byte a partire dal mark attuale del buffer passato come parametro,
 * attraverso struct conversion, per tanti byte quanto il valore di num_bytes.
 * Se il numero di byte da convertire supera la capacita' dell'array contenuto nella struttura,
 * la funzione restituisce il valore neutro 0.
 * @param buffer
 * @param num_bytes
 * @param converter
 * @return
 */
inline int convert(Buffer *buffer, unsigned char num_bytes, Converter *converter)
{
    if (num_bytes > CONVERTER_DIM)
        return 0;
    for (int j = 0; j < num_bytes; ++j) {
        (*converter).byte_arr[j] = *buf_next_elem(buffer);
    }
    return converter->value;
}

/**
 * Interpreta le informazioni del byte di tag,
 * effettua chiamate a funzioni per controllare che le informazioni da leggere siano interamente contenute nel buffer di sorgente,
 * trascrive il contenuto del literal sul buffer di destinazione.
 * @param buf_src
 * @param buf_dest
 * @param source
 * @param converter
 * @return
 */
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

/**
 *
 * @param destination
 * @param source
 * @param len
 */
inline void write_literal(Buffer *destination, Buffer *source, unsigned int len) {
    source->mark++;
    for (unsigned int i = 0; i < len; ++i, destination->mark++, source->mark++) {
        *buf_curr_elem(destination) = *buf_curr_elem(source);
        // printf("%x, ", (unsigned char)*buf_curr_elem(buf_dest));
    }
    source->mark--;
}

/**
 * effettua le chiamate a funzione necessarie a scrivere la copia assumendone un valore di offset,
 * controlla che la lettura delle informazioni siano contenute interamete nel buffer di sorgente
 * e che il valore di offset ottenuto sia plausibile.
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
    // ottiene il valore contenuto nei byte extra
    offset = convert(buf_src, extra_bytes, converter);
    // controllo di plausibilita' del valore di offset
    if (buf_src->mark - offset < 0 || offset > BUFFER_DIM)
        return;
    write_copy(buf_dest, len, offset);
}

/**
 * Trascrive la copia trovata sullo stesso buffer di destinazione, a partire da un valore di offset come parametro in input.
 * @param buffer
 * @param len
 * @param offset
 */
inline void write_copy(Buffer *buffer, int len, unsigned int offset) {
    // leggo a partire dall'offset
    buffer->mark_copy -= offset;
    // copio elemento per elemento
    for (unsigned int i = 0; i < len; ++i, buffer->mark++, buffer->mark_copy++) {
        *buf_curr_elem(buffer) = *buf_rel_elem(buffer);
    }
}

/**
 * Interpreta il tag contenuto nel byte di tag come literal o tipo di copia,
 * modifica situazionalmente i valori delle variabili che diventano parametri per le funzioni associate alle copie.
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

/**
 * Ospita la struttura principale dell'algoritmo di decompressione.
 * Effettua una chiamata a sistema per scrivere sul file di output le informazioni trovate dalle procedure di decompressione.
 * Istanzia i buffer di sorgente e destinazione,
 * controlla che il numero di byte trascritti nel buffer di destinazione
 * raggiunga la dimensione del file non compresso, trovata nei primi byte del file in input.
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