//
// Created by Timothy Turturiello on 30.11.19.
//
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "varint.h"
#include "snappy_decompression.h"
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/test_compressed"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/output_decompressed.txt"
#define BUFFER_DIM 64000

//// converte una sequenza di byte contenuti in un array, in un numero
typedef union convertion {
    char byte_arr[4];
    unsigned long value;
} Converter;

typedef struct buffer {
    char array[BUFFER_DIM];
    long mark;
    long mark_copy;
} Buffer;

void init_buffer(Buffer *buffer);

void init_converter(Converter *converter);

Buffer *buffer_constructor();

int open_resources(FILE **file_in, FILE **file_out);

void close_resources(FILE *file_in, FILE *file_out);

void temp_r_OS(Buffer *buf_src);

void flush(Buffer *buf_out, FILE *infile, unsigned long dim);

void rflush(Buffer *buf_dest, FILE *destination);

void wflush(Buffer *buf_dest, Buffer *buf_src, FILE *destination, FILE *source);

unsigned long decompressor(FILE *destination, FILE *source, Buffer *buf_dest, Buffer *buf_src);

char* buf_curr_elem(Buffer *buffer);

char* buf_next_elem(Buffer *buffer);

float lose_data(FILE *destination, float desire_dim);

int main()
{
    FILE *source;
    FILE *destination;
    if (open_resources(&source, &destination))
        return -1;

    unsigned long uncomp_dim = varint_to_dim(source);
    Buffer *buf_src = (Buffer *) buffer_constructor();
    Buffer *buf_dest = (Buffer *) buffer_constructor();

    // carico il buffer di 64kb del contenuto del file compresso
    flush(buf_src, source, BUFFER_DIM);

    unsigned long readed = 0;
    do {
        readed += decompressor(destination, source, buf_dest, buf_src);
    } while (readed < uncomp_dim);
    wflush(buf_dest, buf_src, destination, source);

    float losing = lose_data(destination, (float) uncomp_dim);
    close_resources(source, destination);
    if (losing) {
        printf("%f%c of loosing data", losing*100,'%');
        return -1;
    }

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

void init_buffer(Buffer *buffer)
{
    buffer->mark = 0;
    memset(buffer->array, 0, BUFFER_DIM);
}

void init_converter(Converter *converter)
{
    converter->value=0;
}

Buffer *buffer_constructor()
{
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    init_buffer(buffer);

    return buffer;
}

void flush(Buffer *buf_out, FILE *infile, unsigned long dim)
{
    fread(buf_out->array, sizeof(char), dim, infile);
}

void wflush(Buffer *buf_dest, Buffer *buf_src, FILE *destination, FILE *source)
{
    // scrivo su file
    fwrite(buf_dest->array, sizeof(char), buf_dest->mark, destination);

    // azzero i campi dei buffer
    init_buffer(buf_src);
    init_buffer(buf_dest);
    // riempio il buffer del contenuto del file compresso
    fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
}

void rflush(Buffer *buf_dest, FILE *destination)
{
    fread(buf_dest->array, sizeof(char), buf_dest->mark, destination);
    init_buffer(buf_dest);
}

char* buf_curr_elem(Buffer *buffer)
{
    return &(buffer->array[buffer->mark]);
}

char* buf_next_elem(Buffer *buffer)
{
    buffer->mark++;
    return &(buffer->array[buffer->mark]);
}

float lose_data(FILE *destination, float desire_dim)
{
    fseek(destination, 0, SEEK_END);
    if (desire_dim != (float)ftell(destination))
        return 1-(float)ftell(destination)/desire_dim;
    return 0;
}

int is_in_buffer(Buffer *buffer, unsigned long n_bytes)
{
    if (buffer->mark + n_bytes  > BUFFER_DIM)
        return 0;
    return 1;
}

void temp_r_OS(Buffer *buf_src)
{
    long ex_mark = buf_src->mark;
    buf_src->mark = 0;
    // trovare e cancellare occorrenze di \r
    int len = BUFFER_DIM;
    int readed = 0;

    do {
        if (*buf_curr_elem(buf_src) == '\r') {
            // shifta tutto fino in fondo
            while (buf_src->mark < len) {
                buf_src->array[buf_src->mark] = buf_src->array[buf_src->mark+1];
                buf_src->mark++;
            }
            // rimetti mark a 0
            buf_src->mark = 0;
        }
        buf_src->mark++;
    } while(buf_src->mark <= len);
    buf_src->mark = ex_mark;
}

unsigned long inline decompressor(FILE *destination, FILE *source, Buffer *buf_dest, Buffer *buf_src)
{
    //TODO: reset temporaneo
    // init_buffer(buf_dest);

    temp_r_OS(buf_src);

    //unsigned char curr_byte = *buf_src->array;
    unsigned char curr_byte = *buf_curr_elem(buf_src);
    //unsigned char curr_byte = *buf_src->array;
    unsigned long memory_mark;
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
    unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
    unsigned char notag_value = (mask_notag&(*buf_curr_elem(buf_src))) >> 2;
    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int offset = 0;
    unsigned int len = 0;
    Converter converter;
    init_converter(&converter);

    char temp;

    unsigned char mode = mask_tag & (*buf_curr_elem(buf_src));
    switch(mode) {
        case 0: // do_literal()
            switch (notag_value) {
                case 63:
                    extra_bytes++;
                case 62:
                    extra_bytes++;
                case 61:
                    extra_bytes++;
                case 60:
                    extra_bytes++;

                    // poblema in lettura
                    // se le informazioni sono sul prossimo buffer -> svuotare il buffer prima di iniziare a leggere
                    if (!is_in_buffer(buf_src, extra_bytes))
                        rflush(buf_src, source);
                    // converto i byte associati alla lunghezza del buffer in valore intero
                    for (int i = extra_bytes; i > 0; i--) {
                        buf_src->mark++;
                        converter.byte_arr[i - 1] = (char)(buf_src->array[buf_src->mark]+1);
                    }
                    len = converter.value;

                    // problema in scrittura
                    // se non è sufficiente la memoria in buf_dest -> svuotare il buffer
                    if (!is_in_buffer(buf_dest, len))
                        wflush(buf_dest, buf_src, destination, source);
                    // copio elemento per elemento
                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++) {
                        buf_src->mark++;
                        *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                    }
                    break;
                default: // <60 len = val+1
                    len = notag_value+1;
                    if (!is_in_buffer(buf_dest, len)) {
                        wflush(buf_dest, buf_src, destination, source);
                    }
                    buf_src->mark++;
                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                        //buf_src->mark++;
                        *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                        temp = *buf_curr_elem(buf_src);
                    }
                    buf_src->mark--;
            }
            break;
            // copie
        case 1:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 1; // di lettura offset
            len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)

            // is_writable
            if (!is_in_buffer(buf_dest, len)) {
                wflush(buf_dest, buf_src, destination, source);
            }
            // bit piu' significativi nel byte di tag
            converter.byte_arr[2] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111

            if (!is_in_buffer(buf_src, extra_bytes)) {
                rflush(buf_src, source);
            }
            // leggo gli extra byte
            converter.byte_arr[3] = *buf_next_elem(buf_src);
            offset = converter.value;

            // leggo a partire dall'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                //buf_src->mark++;
                //*buf_curr_elem(buf_dest) = *buf_elem(buf_src);
                *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];

                temp = *buf_curr_elem(buf_src);
            }
            // buf_src->mark = memory_mark;
            break;
        case 2:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 2;
            len = notag_value+1;

            // copy is_writable
            if (!is_in_buffer(buf_dest, len))
                wflush(buf_dest, buf_src, destination, source);

            // copy is_readable
            if (!is_in_buffer(buf_src, extra_bytes))
                rflush(buf_src, source);

            //buf_src->mark++;
            converter.byte_arr[0] = *buf_next_elem(buf_src);
            //buf_src->mark++;
            //converter.byte_arr[2] = buf_src->array[buf_src->mark];
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            //TODO: fixare offset, potrebbe causare una exception se e' maggiore alla dimensione del buffer,
            // può accadere veramente? O il buf_src converge al contenuto del pacchetto pre-compressione?
            offset = converter.value;
            // offset = 2;

            // memory_mark = buf_src->mark;

            // mi riporto all'inizio dei byte letti e applico l'offset
            //TODO: fixare problemi copia
            // buf_src->mark -= offset;
            buf_dest->mark_copy -= offset;
            // copio elemento per elemento

            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                // *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];

                temp = *buf_curr_elem(buf_src);
            }
            // buf_src->mark = memory_mark;
            break;
        case 3:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 4;
            len = notag_value+1;

            // copy is_writable
            if (!is_in_buffer(buf_dest, len))
                wflush(buf_dest, buf_src, destination, source);

            // copy is_readable
            if (!is_in_buffer(buf_dest, extra_bytes))
                rflush(buf_dest, destination);

            converter.byte_arr[0] = *buf_next_elem(buf_src);
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            converter.byte_arr[2] = *buf_next_elem(buf_src);
            converter.byte_arr[3] = *buf_next_elem(buf_src);
            offset = converter.value;

            // memory_mark = buf_src->mark;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_dest->mark_copy -= offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                // *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];
                temp = *buf_curr_elem(buf_src);
            }
            // buf_src->mark = memory_mark;

            break;
        default:break;
    }
    buf_src->mark++;
    return len;
}