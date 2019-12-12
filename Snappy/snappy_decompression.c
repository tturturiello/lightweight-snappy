//
// Created by Timothy Turturiello on 30.11.19.
//

#include <stdio.h>
#include "snappy_decompression.h"
#include "varint.h"
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/test_compressed"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/output_decompressed.txt"
#define BUFFER_DIM 64000

// 'byte_arr' contiene dei byte che possono essere letti come unico numero attraverso 'value'
typedef union convertion {
    char byte_arr[4];
    int value;
} Converter;

typedef struct source_buffer {
    char buf[BUFFER_DIM];
    long mark;
} Source;

typedef struct destination_buffer {
    char buf[BUFFER_DIM];
    long mark;
} Destination;

int open_resources(FILE **file_in, FILE **file_out);

void close_resources(FILE *file_in, FILE *file_out);

unsigned int decompressor(Destination *dest_buf, Source *src_buf);

int main() {
    FILE *source;
    FILE *destination;
    open_resources(&source, &destination);

    unsigned long uncomp_dim = varint_to_dim(source);
    Source *buf_src = (Source *)malloc(sizeof(Source));
    Destination *buf_dest = (Destination *)malloc(sizeof(Destination));

    fread(buf_src->buf, sizeof(char), BUFFER_DIM, source);

    //char pack_buf[BUFFER_DIM];
    int out = 0;
    unsigned long readed = 0;
    unsigned long readed_tot = 0;

    // finche' non ho letto tutto il buffer src_buf
    while (out<3) {
        // voglio che il buffer si riempia totalmente non perdendo il puntatore alla chiamata precedente
        readed = decompressor(buf_dest, buf_src);
        readed_tot+=readed;
        out++;
    }
    // scrivo su file
    fwrite(buf_dest->buf, sizeof(char), readed_tot, destination);
    close_resources(source, destination);
    return 0;
}

int open_resources(FILE **file_in, FILE **file_out) {
    if ((*file_in = fopen(FINPUT_NAME, "rb"))== NULL) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
    if ((*file_out = fopen(FOUTPUT_NAME, "wb")) == NULL ) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
}

void close_resources(FILE *file_in, FILE *file_out) {
    fclose(file_in);
    fclose(file_out);
}

unsigned int decompressor(Destination *dest_buf, Source *src_buf)
{
    unsigned char byte_buf = *src_buf->buf;
    unsigned char debug = *dest_buf->buf;

    // fread(&byte_buf, sizeof(char), 1, source); // inserisce il prossimo byte nel byte_buf

    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
    unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
    unsigned char notag_value = (mask_notag&byte_buf)>>2;
    unsigned char assoc_bytes = 0; // numero di byte associati
    unsigned int offset = 0;
    unsigned int temp_offset = 0;

    unsigned int len = 0;
    Converter converter;

    //fread(&byte_buf, sizeof(char),1, source);


    unsigned char mode = mask_tag&byte_buf;
    switch(mode) {
        case 0: // do_literal()
            switch (notag_value) {
                case 63:
                    assoc_bytes++;
                case 62:
                    assoc_bytes++;
                case 61:
                    assoc_bytes++;
                case 60:
                    assoc_bytes++;

                    // TODO: capire come aumentare il puntatore alla struttura come in file.
                    // trasformo i byte associati alla lunghezza del buffer in valore intero
                    for (int i = assoc_bytes; i > 0; i--, src_buf->mark++) {
                        converter.byte_arr[i - 1] = src_buf->buf[src_buf->mark];
                    }
                    len = converter.value + 1;

                    // copio elemento per elemento
                    for (unsigned int i = 0; i < len; ++i, dest_buf->mark++, src_buf->mark++) {
                        dest_buf->buf[dest_buf->mark] = src_buf->buf[src_buf->mark];
                    }
                    break;
                default: // <60 len = val+1
                    len = notag_value+1;
                    for (unsigned int i = 0; i < len; ++i, dest_buf->mark++, src_buf->mark++) {
                        dest_buf->buf[dest_buf->mark] = src_buf->buf[src_buf->mark];
                    }
            }
            break;
            // copie
        case 1:
            // similmente al literal ho al massimo 4 byte da riempire con il valore dell'offset
            len = ((byte_buf&mask_dx_notag)>>2)+4; // 00011100 -> 111 + 4
            // bit piu' significativi nel byte di tag
            converter.byte_arr[2] = (byte_buf & mask_sx_notag) >> 5; // 11100000 -> 111
            //TODO: ERI RIMASTO QUI
            converter.byte_arr[3] = src_buf->buf[src_buf->mark++];
            offset = converter.value;

            // mi riporto prima dell'offset letto e all'inizio del byte
            src_buf -= 1+offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, dest_buf->mark++, src_buf->mark++) {
                dest_buf->buf[dest_buf->mark] = src_buf->buf[src_buf->mark];
            }
            break;
        case 2:
            len = notag_value+1;
            converter.byte_arr[3] = src_buf->buf[src_buf->mark++];
            converter.byte_arr[2] = src_buf->buf[src_buf->mark++];
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            src_buf -= 2+offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, dest_buf->mark++, src_buf->mark++) {
                dest_buf->buf[dest_buf->mark] = src_buf->buf[src_buf->mark];
            }
            break;
        case 3:
            // prossimi 4 byte
            len = notag_value+1;
            converter.byte_arr[3] = src_buf->buf[src_buf->mark++];
            converter.byte_arr[2] = src_buf->buf[src_buf->mark++];
            converter.byte_arr[1] = src_buf->buf[src_buf->mark++];
            converter.byte_arr[0] = src_buf->buf[src_buf->mark++];
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            src_buf -= 4+offset;
            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, dest_buf->mark++, src_buf->mark++) {
                dest_buf->buf[dest_buf->mark] = src_buf->buf[src_buf->mark];
            }
            break;
        default:break;
    }
    return len;
}
