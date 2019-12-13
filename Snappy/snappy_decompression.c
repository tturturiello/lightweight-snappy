//
// Created by Timothy Turturiello on 30.11.19.
//

#include <stdio.h>
#include "snappy_decompression.h"
#include "varint.h"
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/test_compressed"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/output_decompressed.txt"
#define BUFFER_DIM 64000 // caso peggiore: nessuna compressione, 5 byte di descrizione

//// converte una sequenza di byte in un array, in un numero
typedef union convertion {
    char byte_arr[4];
    unsigned long value;
} Converter;

typedef struct buffer {
    char array[BUFFER_DIM];
    long mark;
} Buffer;

void init_buffer(Buffer *buffer);

int open_resources(FILE **file_in, FILE **file_out);

void close_resources(FILE *file_in, FILE *file_out);

void flush(Buffer *buffer, FILE *file, char mode);

void decompressor(FILE *destination, FILE *source, Buffer *buf_dest, Buffer *buf_src);

char buf_curr_elem(Buffer *buffer);

float lose_data(FILE *destination, float desire_dim);

int main()
{
    FILE *source;
    FILE *destination;
    if (open_resources(&source, &destination))
        return -1;

    unsigned long uncomp_dim = varint_to_dim(source);
    Buffer *buf_src = (Buffer *)malloc(sizeof(Buffer));
    Buffer *buf_dest = (Buffer *)malloc(sizeof(Buffer));
    init_buffer(buf_src);
    init_buffer(buf_dest);

    // carico il buffer di 64kb del contenuto del file compresso
    fread(buf_src->array, sizeof(char), BUFFER_DIM, source);

    // fino alla fine del file
    do {
        decompressor(destination, source, buf_dest, buf_src);
    } while (buf_curr_elem(buf_src) != '\0');
    flush(buf_dest, destination, 'w');

    float losing = lose_data(destination, (float) uncomp_dim);
    if (losing) {
        printf("%f%c of loosing data", losing*100, '%');
        close_resources(source, destination);
        return -1;
    }
    close_resources(source, destination);

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
}

void close_resources(FILE *file_in, FILE *file_out)
{
    fclose(file_in);
    fclose(file_out);
}

void init_buffer(Buffer *buffer)
{
    buffer->mark = 0;
}

void flush(Buffer *buffer, FILE *file, char mode)
{
    switch(mode) {
        case 'r': fread(buffer->array, sizeof(char), buffer->mark, file);
        case 'w': fwrite(buffer->array, sizeof(char), buffer->mark, file);
        default:break;
    }
}

char buf_curr_elem(Buffer *buffer)
{
    return buffer->array[buffer->mark];
}

float lose_data(FILE *destination, float desire_dim)
{
    fseek(destination, 0, SEEK_END);
    if (desire_dim != (float)ftell(destination))
        return (float)ftell(destination)/desire_dim;
    return 0;
}

void inline decompressor(FILE *destination, FILE *source, Buffer *buf_dest, Buffer *buf_src)
{
    unsigned char byte_buf = *buf_src->array;
    unsigned char debug = *buf_dest->array;

    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
    unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
    unsigned char notag_value = (mask_notag&byte_buf)>>2;
    unsigned char extra_bytes = 0; // numero di byte associati
    long offset = 0;
    unsigned long len = 0;
    Converter converter;

    //fread(&byte_buf, sizeof(char),1, source);
    //TODO: usare un array di puntatore a funzioni da incrementare da un case a un altro

    unsigned char mode = mask_tag&byte_buf;
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
                    if (extra_bytes + buf_src->mark > BUFFER_DIM)
                        flush(buf_src, source, 'r');
                    // TODO: capire come aumentare il puntatore alla struttura Buffer come in file.
                    // converto i byte associati alla lunghezza del buffer in valore intero
                    for (int i = extra_bytes; i > 0; i--, buf_src->mark++) {
                        converter.byte_arr[i - 1] = buf_src->array[buf_src->mark];
                    }
                    len = converter.value + 1;

                    // problema in scrittura
                    // se non è sufficiente la memoria in buf_dest -> svuotare il buffer
                    if (len + buf_dest->mark > BUFFER_DIM)
                        flush(buf_dest, destination, 'w');
                    // copio elemento per elemento
                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                        buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
                    }
                    break;
                default: // <60 len = val+1
                    len = notag_value+1;
                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                        buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
                    }
            }
            break;
            // copie
        case 1:
            // similmente al literal ho al massimo 4 byte da riempire con il valore dell'offset
            extra_bytes = 1;
            len = ((byte_buf&mask_dx_notag)>>2)+4; // 00011100 -> 111 + 4 (lungheza copia)

            // is_writable
            if (len + buf_dest->mark > BUFFER_DIM)
                flush(buf_dest, destination, 'w');

            // bit piu' significativi nel byte di tag
            converter.byte_arr[2] = (byte_buf & mask_sx_notag) >> 5; // 11100000 -> 111

            if (extra_bytes + buf_src->mark > BUFFER_DIM)
                flush(buf_src, source, 'r');
            converter.byte_arr[3] = buf_src->array[buf_src->mark++];
            offset = converter.value;

            // mi riporto prima dell'offset letto e all'inizio del byte
            buf_src -= 1 + offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
            }
            break;
        case 2:
            extra_bytes = 2;
            len = notag_value+1;

            // copy is_writable
            if (len + buf_dest->mark > BUFFER_DIM)
                flush(buf_dest, destination, 'w');

            // copy is_readable
            if (extra_bytes + buf_dest->mark > BUFFER_DIM)
                flush(buf_dest, destination, 'w');

            converter.byte_arr[3] = buf_src->array[buf_src->mark++];
            converter.byte_arr[2] = buf_src->array[buf_src->mark++];
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_src -= 2 + offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
            }
            break;
        case 3:
            extra_bytes = 4;
            len = notag_value+1;

            // copy is_writable
            if (len + buf_dest->mark > BUFFER_DIM)
                flush(buf_dest, destination, 'w');

            // copy is_readable
            if (extra_bytes + buf_dest->mark > BUFFER_DIM)
                flush(buf_dest, destination, 'w');

            converter.byte_arr[3] = buf_src->array[buf_src->mark++];
            converter.byte_arr[2] = buf_src->array[buf_src->mark++];
            converter.byte_arr[1] = buf_src->array[buf_src->mark++];
            converter.byte_arr[0] = buf_src->array[buf_src->mark++];
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_src -= 4 + offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                buf_dest->array[buf_dest->mark] = buf_src->array[buf_src->mark];
            }
            break;
        default:break;
    }
}
