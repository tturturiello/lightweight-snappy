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
//#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/test_compressed"
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/alice_compressed"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/output_decompressed.txt"

// #define BUFFER_DIM 65536+5 // worst case: nessuna compressione e 5 byte per esprimere copia o literal
#define BUFFER_DIM 4000000
// #define BUFFER_DIM 100

// TODO: attenzione a lavorare col contenuto del puntatore di dest_buf

//// converte una sequenza di byte contenuti in un array, in un numero
union convertion {
    char byte_arr[4];
    unsigned long value;
};


struct buffer {
    char array[BUFFER_DIM];
    long mark;
    long mark_copy;
};


struct source {
    char array[BUFFER_DIM];
    unsigned long mark;
};

struct destination {
    char* array;
    unsigned long mark;
    unsigned long mark_copy;
};

union bufferoni {
    Buffer *src;
    struct destination dest;
};

void init_buffer(Buffer *buffer);

void init_converter(Converter *converter);

Buffer *buffer_constructor();

Source_buffer *buf_src_constructor();

Destination_buffer *buf_dest_constructor(unsigned long dimension);

int is_readable(Source_buffer *buffer, unsigned long n_bytes);

int open_resources(FILE **file_in, FILE **file_out);

void close_resources(FILE *file_in, FILE *file_out);

void temp_r_OS(Buffer *buf_src);

void flush(Buffer *buf_out, FILE *infile, unsigned long dim);

void rflush(Destination_buffer *buf_dest, FILE *destination);

void wflush(Destination_buffer *buf_dest, Buffer *buf_src, FILE *destination, FILE *source);

unsigned long decompressor(FILE *destination, FILE *source, Destination_buffer *buf_dest, Source_buffer *buf_src);

char* buf_curr_elem(Buffer *buffer);

char* buf_next_elem(Source_buffer *buffer);

float lose_data(FILE *destination, float desire_dim);

int decompress(FILE *finput, FILE *fdecompressed)
{
    unsigned long uncomp_dim = varint_to_dim(finput);

    Source_buffer *buf_src = (Source_buffer*)malloc(sizeof(Source_buffer));
    buf_src->mark=0;
    memset(buf_src->array, 0, BUFFER_DIM);


    Destination_buffer *buf_dest = (Destination_buffer*)malloc(sizeof(Destination_buffer));
    char container[uncomp_dim];
    memset(container, 0, uncomp_dim);
    buf_dest->mark=0;
    buf_dest->mark_copy=0;
    buf_dest->array = container;

    fread(buf_src->array, sizeof(char), BUFFER_DIM, finput);

    int count = 0;
    unsigned long readed = 0;
    do {
        count++;
        printf("%d)", count);
        readed += decompressor(fdecompressed, finput, buf_dest, buf_src);

    } while (readed < uncomp_dim);

    // scrivo il contenuto del buffer principale nel file
    fwrite(container, sizeof(char), uncomp_dim, fdecompressed);

    close_resources(finput, fdecompressed);

    return 0;
}

int main()
{
    FILE *source;
    FILE *destination;
    if (open_resources(&source, &destination))
        return -1;

    unsigned long uncomp_dim = varint_to_dim(source);

    //Buffer *buf_src = (Buffer *) buffer_constructor();
    // Buffer *buf_dest = (Buffer *) buffer_constructor();

    //Source_buffer *buf_src = (Source_buffer*) buf_src_constructor();
    Source_buffer *buf_src = (Source_buffer*)malloc(sizeof(Source_buffer));
    buf_src->mark=0;
    memset(buf_src->array, 0, BUFFER_DIM);


    //Destination_buffer *buf_dest = (Destination_buffer*) buf_dest_constructor(uncomp_dim);
    Destination_buffer *buf_dest = (Destination_buffer*)malloc(sizeof(Destination_buffer));
    char container[uncomp_dim];
    memset(container, 0, uncomp_dim);
    buf_dest->mark=0;
    buf_dest->mark_copy=0;
    buf_dest->array = container;

    // carico il buffer di 64kb del contenuto del file compresso
    //flush(buffers.src, source, BUFFER_DIM);
    fread(buf_src->array, sizeof(char), BUFFER_DIM, source);

    int count = 0;
    unsigned long readed = 0;
    do {
        count++;
        printf("%d)", count);
        readed += decompressor(destination, source, buf_dest, buf_src);

    } while (readed < uncomp_dim);

    //wflush(buf_dest, buf_src, destination, source);
    // scrivo il contenuto del buffer principale nel file
    fwrite(container, sizeof(char), uncomp_dim, destination);



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

void reset_buffer(Buffer *buffer)
{
    buffer->mark = 0;
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

Source_buffer *buf_src_constructor()
{
    Source_buffer *buffer = (Source_buffer*)malloc(sizeof(Source_buffer));
    buffer->mark=0;
    memset(buffer->array, 0, BUFFER_DIM);
    return buffer;
}

Destination_buffer *buf_dest_constructor(unsigned long dimension)
{
    Destination_buffer *dest = (Destination_buffer*)malloc(sizeof(Destination_buffer));
    char array[dimension];
    memset(array, 0, dimension);
    dest->mark=0;
    dest->mark_copy=0;
    dest->array = array;
    //char *array = (char*)malloc(sizeof(dimension));
    //buffer->array = (char*)malloc(sizeof(dimension));

    return dest;
}

void flush(Buffer *buf_out, FILE *infile, unsigned long dim)
{
    fread(buf_out->array, sizeof(char), dim, infile);
}

void wflush(Destination_buffer *buf_dest, Buffer *buf_src, FILE *destination, FILE *source)
{
    // scrivo su file
    fwrite(buf_dest->array, sizeof(char), buf_dest->mark, destination);

    // azzero i campi dei buffer
    reset_buffer(buf_src);
    reset_buffer(buf_dest);

    // riempio il buffer del contenuto del file compresso
    fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
}

void rflush(Destination_buffer *buf_dest, FILE *destination)
{
    fread(buf_dest->array, sizeof(char), buf_dest->mark, destination);
    reset_buffer(buf_dest);
}

char* buf_curr_elem(Buffer *buffer)
{
    return &(buffer->array[buffer->mark]);
}

char* buf_next_elem(Source_buffer *buffer)
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

int is_readable(Source_buffer *buffer, unsigned long n_bytes)
{
    // problemi in lettura
    if (buffer->mark + n_bytes >= BUFFER_DIM)
        return 0;
    return 1;
}

/*
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
*/


unsigned long inline decompressor(FILE *destination, FILE *source, Destination_buffer *buf_dest, Source_buffer *buf_src)
{
    //TODO: reset temporaneo
    // init_buffer(buf_dest);

    //unsigned char curr_byte = *buf_src->array;
    unsigned char curr_byte = (buf_src->array[buf_src->mark]); // corrente
    //unsigned char curr_byte = *buf_src->array;
    unsigned long memory_mark;
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
    unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
    unsigned char notag_value = (mask_notag&(curr_byte)) >> 2;
    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int offset = 0;
    unsigned int len = 0;
    Converter converter;
    init_converter(&converter);

    unsigned char mode = mask_tag & (curr_byte);
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
                    /*
                    if (!is_in_buffer(buf_src, extra_bytes)) {
                        rflush(buf_src, source);
                    }
                    */
                    if (!is_readable(buf_src, extra_bytes)) {
                        for (int j = 0; j < BUFFER_DIM; ++j) {
                            buf_src->array[j] = buf_dest->array[buf_dest->mark];
                            buf_dest->mark++;
                        }
                        buf_src->mark = 0;
                    }

                    // converto i byte associati alla lunghezza del buffer in valore intero
                    for (int i = extra_bytes; i > 0; i--) {
                        buf_src->mark++;
                        converter.byte_arr[i - 1] = (char)(buf_src->array[buf_src->mark]+1);
                    }
                    len = converter.value;

                    // copio elemento per elemento
                    unsigned long i;
                    for (i = 0; i < len; ++i, buf_dest->mark++) {
                        buf_src->mark++;
                        //*buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                        // ((*buf_dest).array)[2] = buf_src->array[buf_src->mark];
                        buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
                    }
                    break;
                default: // <60 len = val+1
                    len = notag_value+1;
                    buf_src->mark++;

                    if (!is_readable(buf_src, extra_bytes)) {
                        // fread(buf_src->array, sizeof(char), BUFFER_DIM, destination);
                        for (int j = 0; j < BUFFER_DIM; ++j) {
                            buf_src->array[j] = buf_dest->array[buf_dest->mark];
                            buf_dest->mark++;
                        }
                        buf_src->mark = 0;
                    }

                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                        //*buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                        buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
                    }
                    buf_src->mark--;
            }
            printf(" L,  len %d\n", len);
            break;
            // copie
        case 1:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 1; // di lettura offset
            len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)

            // bit piu' significativi nel byte di tag
            converter.byte_arr[1] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111

            if (!is_readable(buf_src, extra_bytes)) {
                for (int j = 0; j < BUFFER_DIM; ++j) {
                    buf_src->array[j] = buf_dest->array[buf_dest->mark];
                    buf_dest->mark++;
                }
                buf_src->mark = 0;
            }
            // leggo gli extra byte
            converter.byte_arr[0] = *buf_next_elem(buf_src);
            offset = converter.value;

            // leggo a partire dall'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                //buf_src->mark++;
                //*buf_curr_elem(buf_dest) = *buf_elem(buf_src);
                // *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];

                buf_dest->array[buf_dest->mark] = buf_dest->array[buf_dest->mark_copy];
                //*((*buf_dest).array) = buf_src->array[buf_src->mark];
            }
            printf(" 01, len %d, off %d \n", len, offset);
            break;
        case 2:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 2;
            len = notag_value+1;

            // copy is_readable
            if (!is_readable(buf_src, extra_bytes)) {
                for (int j = 0; j < BUFFER_DIM; ++j) {
                    buf_src->array[j] = buf_dest->array[buf_dest->mark];
                    buf_dest->mark++;
                }
                buf_src->mark = 0;
            }

            converter.byte_arr[0] = *buf_next_elem(buf_src);
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            //TODO: fixare offset, potrebbe causare una exception se e' maggiore alla dimensione del buffer,
            // può accadere veramente? O il buf_src converge al contenuto del pacchetto pre-compressione?
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                // *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];
                buf_dest->array[buf_dest->mark] = buf_dest->array[buf_dest->mark_copy];
            }
            // buf_src->mark = memory_mark;
            printf(" 10, len %d, off %d \n", len, offset);
            break;
        case 3:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 4;
            len = notag_value+1;

            // copy is_readable
            if (!is_readable(buf_src, extra_bytes)) {
                for (int j = 0; j < BUFFER_DIM; ++j) {
                    buf_src->array[j] = buf_dest->array[buf_dest->mark];
                    buf_dest->mark++;
                }
                buf_src->mark = 0;
            }

            converter.byte_arr[0] = *buf_next_elem(buf_src);
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            converter.byte_arr[2] = *buf_next_elem(buf_src);
            converter.byte_arr[3] = *buf_next_elem(buf_src);

            offset = converter.value;


            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                // *buf_curr_elem(buf_dest) = buf_dest->array[buf_dest->mark_copy];
                buf_dest->array[buf_dest->mark] = buf_dest->array[buf_dest->mark_copy];
            }

            printf(" 11, len %d, off %d \n", len, offset);
            break;
        default:break;
    }
    buf_src->mark++;
    return len;
}