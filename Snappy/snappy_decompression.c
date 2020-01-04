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
//#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Decompressed_test/alice_compressed.txt"

#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Standard_test/50000b5.snp"
#define FOUTPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Standard_test/50000b5dec"

//#define FINPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Compressed_test\\alice_compressed.snp"
//#define FOUTPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Decompressed_test\\alice_decompressed.txt"

#define BUFFER_DIM 65536

// TODO: stampare risultati decompressione .csv o .txt (generare)
// info:
// dim input
// dim output
// tempo
// mb/sec

/*
//// converte una sequenza di byte contenuti in un array, in un numero
union convertion {
    char byte_arr[4];
    unsigned long value;
};

struct buffer {
    char* array;
    unsigned long mark;
    unsigned long mark_copy;
};
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

/*
struct test {
    unsigned long long dim_input;
    unsigned long long dim_output;
    float time;
    float mbps;
};
 */
static unsigned long long dim_input;
static unsigned long long dim_output;
static float t;
static float mbps;


void test();

void init_buffer(Buffer *buffer, const unsigned long *buf_dim, char container[*buf_dim]);

void init_converter(Converter *converter);

Buffer *buffer_constructor();

int is_readable(Buffer *buffer, unsigned long n_bytes, unsigned int length);

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

void do_literal(Buffer *buf_src, Buffer *buf_dest, FILE *source);

void do_copy(Buffer *buf_src, Buffer *buf_dest, FILE *source, int mode);

void do_copy_01(Buffer *buf_src, Buffer *buf_dest, FILE *source);

unsigned long long get_file_size(FILE *file);

#endif

static double time_taken;
 int main()
{
    clock_t time = clock();

    FILE *source;
    FILE *destination;
    if (open_resources(&source, &destination))
        return -1;

    unsigned long uncomp_dim = varint_to_dim(source);

    // TODO: generalizzare attraverso dei costruttori
    // Buffer *buf_src = (Buffer *) buffer_constructor(BUFFER_DIM);
    // Buffer *buf_dest = (Buffer *) buffer_constructor(uncomp_dim);

    Buffer *buf_src = (Buffer*)malloc(sizeof(Buffer));
    char container_0[BUFFER_DIM];
    buf_src->mark=0;
    buf_src->mark_copy=0;
    buf_src->array = container_0;

    Buffer *buf_dest = (Buffer*)malloc(sizeof(Buffer));
    char container[uncomp_dim];
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
        readed += decompressor(source, buf_dest, buf_src);

    } while (readed < uncomp_dim);

    // wflush(buf_dest, buf_src, destination, source);
    // scrivo il contenuto del buffer principale nel file
    fwrite(buf_dest->array, sizeof(char), uncomp_dim, destination);

    time = clock() - time;
    double time_taken = ((double)time)/CLOCKS_PER_SEC;
    //printf("tempo di esecuzione: %ld",time_taken);

    print_result_decompression(get_file_size(destination), get_file_size(source));
    // test();
    return 0;
}


/*
void test()
{
    unsigned int dim[] ={100, 200, 500, 1000, 2000, 5000, 10000, 50000, 100000, 200000, 500000, 1000000, 2000000};
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Test *test = malloc(sizeof(Test));

    char finput_name[300];
    char fcompressed_name[300];
    char fdecompressed_name[300];
    char fcsv_name[300];


    FILE *finput;
    FILE *fcompressed;
    FILE *fdecompressed;
    FILE *fcsv;

    // unsigned long csv_dim = len(dim)*300;
    // int dim_0 =  *(&dim + 1) - dim;
    //sprintf(fcsv_name, "..\\Standard_test\\%ub%d.csv", csv_dim);

    char *(*str_ptr)[300] = NULL; // per i nomi: array di putatori a stringhe (fdecompressed_name si aggiorna)
    unsigned long long dim_input[300] = {0};
    unsigned long long dim_output[300] = {0};
    float time[300] = {0};
    float mbps[300] = {0};

    // fai tre buffer:
    // nomi
    // dim_input
    // dim_output
    // tempo
    // mb/sec

    fcsv = fopen("/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Support_output_decompression/decompression_data.txt", "wb");
    //assert(fcsv != NULL);

    int count = 0;
    int max_i = 3;
    int max_j = 15;
    for (int i = 0; i < max_i; ++i) {
        for (int j = 1; j <= max_j; ++j) {
            sprintf(finput_name, "../Standard_test/%ub%d.snp", dim[i], j);
            sprintf(fdecompressed_name,
                    "../Support_output_decompression/%ub%d_dec", dim[i], j);

            finput = fopen(finput_name, "rb");
            //assert(finput != NULL);
            fdecompressed = fopen(fdecompressed_name, "wb");
            //assert(fdecompressed != NULL);

            snappy_decompress(finput, fdecompressed);

            if ((fdecompressed = fopen(fdecompressed_name, "rb")) != NULL) {
                print_result_decompression(get_file_size(fdecompressed), get_file_size(finput));
            }


            dim_input[count] = test->dim_input;
            dim_output[count] = test->dim_output;
            time[count] = test->time;
            mbps[count] = test->mbps;


            fclose(finput);
            fclose(fdecompressed);

            printf("------------------------------------------------------\n\n");
            count++;
        }
    }

    const int buf_dim = 30000;
    char buff[buf_dim];
    memset(buff, '\0', sizeof(buff));
    setvbuf(stdout, buff, _IOFBF, buf_dim);

    // TODO: nomi
    for (int k = 0; k < count-1; ++k) {
        ;;
    }
    printf("\n");
    for (int k = 0; k < count-1; ++k) {
        printf("%llu, ",dim_input[k]);
    }
    printf("\n");
    for (int k = 0; k < count-1; ++k) {
        printf("%llu, ",dim_output[k]);
    }
    printf("\n");
    for (int k = 0; k < count-1; ++k) {
        printf("%f, ",time[k]);
    }
    printf("\n");
    for (int k = 0; k < count-1; ++k) {
        printf("%f, ",mbps[k]);
    }
    fwrite(buff, sizeof(char), buf_dim, fcsv);

    fclose(fcsv);
}
 */

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

void reset_buffer(Buffer *buffer)
{
    buffer->mark = 0;
}

void init_converter(Converter *converter)
{
    converter->value=0;
}

void init_buffer(Buffer *buffer, const unsigned long* buf_dim, char container[*buf_dim])
{
    container = (char *) malloc(sizeof(*buf_dim));
    buffer->mark = 0;
    buffer->mark_copy = 0;
    buffer->array = container;
}

Buffer *buffer_constructor(unsigned long buf_dim)
{
    Buffer *buffer = (Buffer*)malloc(sizeof(Buffer));
    char container[buf_dim];
    init_buffer(buffer, &buf_dim, container);

    return buffer;
}

void flush(Buffer *buf_out, FILE *infile, unsigned long dim)
{
    fread(buf_out->array, sizeof(char), dim, infile);
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

void do_literal(Buffer *buf_src, Buffer *buf_dest, FILE *source)
{
    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int len = 0;

    unsigned char curr_byte = *buf_curr_elem(buf_src); // corrente
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char notag_value = (mask_notag&(curr_byte)) >> 2;

    Converter converter;
    init_converter(&converter);
    switch (notag_value) {
        case 63:
            extra_bytes++;
        case 62:
            extra_bytes++;
        case 61:
            extra_bytes++;
        case 60:
            extra_bytes++;

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, -(BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
                buf_src->mark = 0;
            }

            // converto i byte associati alla lunghezza del buffer in valore intero
            for (int i = extra_bytes; i > 0; i--) {
                buf_src->mark++;
                converter.byte_arr[i - 1] = (char) (buf_src->array[buf_src->mark] + 1);
            }
            len = converter.value;

            // copio elemento per elemento
            for (int i = 0; i < len; ++i, buf_dest->mark++) {
                buf_src->mark++;
                *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
            }
            break;
        default: // <60 len = val+1
            len = notag_value + 1;
            buf_src->mark++;

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, -(BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
                buf_src->mark = 0;
            }

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
            }
            buf_src->mark--;
    }
}

void do_copy(Buffer *buf_src, Buffer *buf_dest, FILE *source, int mode)
{
    buf_dest->mark_copy = buf_dest->mark;
    Converter converter;
    init_converter(&converter);

    unsigned char curr_byte = *buf_curr_elem(buf_src); // corrente
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char notag_value = (mask_notag&(curr_byte)) >> 2;

    unsigned char extra_bytes = 0; // numero di byte associati
    unsigned int offset = 0;
    unsigned int len = 0;

    switch (mode) {
        case 1:
            extra_bytes = 1; // di lettura offset
            unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
            unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
            len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)

            // bit piu' significativi nel byte di tag
            converter.byte_arr[1] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
                buf_src->mark = 0;
            }
            // leggo gli extra byte
            converter.byte_arr[0] = *buf_next_elem(buf_src);
            offset = converter.value;

            // leggo a partire dall'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
            }
        case 2:
            extra_bytes = 2;
            len = notag_value+1;
            buf_dest->mark_copy = buf_dest->mark;

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
                buf_src->mark = 0;
            }

            converter.byte_arr[0] = *buf_next_elem(buf_src);
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);

            }
        case 3:
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 4;
            len = notag_value+1;

            // copy is_readable
            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
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
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
            }
            break;
        default:break;
    }
}

void do_copy_01(Buffer *buf_src, Buffer *buf_dest, FILE *source)
{
    unsigned char curr_byte = *buf_curr_elem(buf_src); // byte corrente

    unsigned char mask_dx_notag = 0x1C;
    unsigned char mask_sx_notag = 0xE0;

    unsigned char extra_bytes = 1; // numero di byte associati
    unsigned int offset;
    unsigned int len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)

    Converter converter;
    init_converter(&converter);

    // bit piu' significativi nel byte di tag
    converter.byte_arr[1] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111

    if (!is_readable(buf_src, extra_bytes, len)) {
        fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
        fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
        buf_src->mark = 0;
    }
    // leggo gli extra byte
    converter.byte_arr[0] = *buf_next_elem(buf_src);
    offset = converter.value;

    // leggo a partire dall'offset
    buf_dest->mark_copy -= offset;

    // copio elemento per elemento
    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
        *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
    }
}

unsigned long inline decompressor(FILE *source, Buffer *buf_dest, Buffer *buf_src)
{
    static unsigned long counter;
    //unsigned char curr_byte = (buf_src->array[buf_src->mark]); // corrente
    unsigned char curr_byte = *buf_curr_elem(buf_src); // corrente
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
        case 0: //do_literal(buf_src, buf_dest, source); break;
            switch (notag_value) {
                case 63:
                    extra_bytes++;
                case 62:
                    extra_bytes++;
                case 61:
                    extra_bytes++;
                case 60:
                    extra_bytes++;

                    if (!is_readable(buf_src, extra_bytes, len)) {
                        fseek(source, -(BUFFER_DIM - buf_src->mark), 1);
                        fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
                        buf_src->mark = 0;
                    }


                    // converto i byte associati alla lunghezza del buffer in valore intero
                    for (int i = 0; i < extra_bytes; i++) {
                        buf_src->mark++;
                        converter.byte_arr[i] = (char) (buf_src->array[buf_src->mark]);

                    }
                    len = converter.value + 1;

                    // copio elemento per elemento
                    for (int i = 0; i < len; ++i, buf_dest->mark++) {
                        buf_src->mark++;
                        *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                    }
                    break;
                default: // <60 len = val+1
                    len = notag_value + 1;
                    buf_src->mark++;

                    if (!is_readable(buf_src, extra_bytes, len)) {
                        fseek(source, -(BUFFER_DIM - buf_src->mark), 1);
                        fread(buf_src->array, sizeof(char), BUFFER_DIM, source);
                        buf_src->mark = 0;
                    }

                    // copio elemento per elemento
                    for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_src->mark++) {
                        *buf_curr_elem(buf_dest) = *buf_curr_elem(buf_src);
                    }
                    buf_src->mark--;
            }
            break;
        case 1: // do_copy_01(buf_src, buf_dest, source); break;
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 1; // di lettura offset
            len = ((curr_byte & mask_dx_notag) >> 2) + 4; // 00011100 -> 111 + 4 (lungheza copia)

            // bit piu' significativi nel byte di tag
            converter.byte_arr[1] = (curr_byte & mask_sx_notag) >> 5; // 11100000 -> 111

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
                buf_src->mark = 0;
            }
            // leggo gli extra byte
            converter.byte_arr[0] = *buf_next_elem(buf_src);
            offset = converter.value;

            // leggo a partire dall'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
            }
            break;
        case 2: //do_copy(buf_src, buf_dest, source, 2); break;
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 2;
            len = notag_value+1;

            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
                buf_src->mark = 0;
            }

            converter.byte_arr[0] = *buf_next_elem(buf_src);
            converter.byte_arr[1] = *buf_next_elem(buf_src);
            offset = converter.value;

            // mi riporto all'inizio dei byte letti e applico l'offset
            buf_dest->mark_copy -= offset;

            // copio elemento per elemento
            for (unsigned int i = 0; i < len; ++i, buf_dest->mark++, buf_dest->mark_copy++) {
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
            }
            break;
        case 3: //do_copy(buf_src, buf_dest, source, 3); break;
            buf_dest->mark_copy = buf_dest->mark;

            extra_bytes = 4;
            len = notag_value+1;

            // copy is_readable
            if (!is_readable(buf_src, extra_bytes, len)) {
                fseek(source, - (BUFFER_DIM - buf_src->mark), 1);
                fread(buf_src->array, sizeof(char),BUFFER_DIM,source);
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
                *buf_curr_elem(buf_dest) = *buf_rel_elem(buf_dest);
            }
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

int snappy_decompress(FILE *file_input, FILE *file_decompressed)
{
    clock_t time = clock();
    unsigned long uncomp_dim = varint_to_dim(file_input);

    Buffer *buf_src = (Buffer*)malloc(sizeof(Buffer));
    char container_0[BUFFER_DIM];
    buf_src->mark=0;
    buf_src->mark_copy=0;
    buf_src->array = container_0;

    Buffer *buf_dest = (Buffer*)malloc(sizeof(Buffer));
    char container[uncomp_dim];
    buf_dest->mark=0;
    buf_dest->mark_copy=0;
    buf_dest->array = container;

    fread(buf_src->array, sizeof(char), BUFFER_DIM, file_input);

    int count = 0;
    unsigned long readed = 0;
    do {
        count++;
        readed += decompressor(file_input, buf_dest, buf_src);

    } while (readed < uncomp_dim);

    // scrivo il contenuto del buffer principale nel file
    fwrite(buf_dest->array, sizeof(char), uncomp_dim, file_decompressed);
    time = clock() - time;
    t = time;
    time_taken = ((double)time)/CLOCKS_PER_SEC;
    dim_input = get_file_size(file_input);
    dim_output = get_file_size(file_decompressed);
    mbps = dim_output/(time_taken * 1e6);

    free(buf_src);
    free(buf_dest);
    return 0;
}

void print_result_decompression(unsigned long fdecompressed_size, unsigned long fcompressed_size)
{
    // TODO: stampare risultati decompressione .csv o .txt (generare)
    // info:
    // dim input
    // dim output
    // tempo
    // mb/sec

    printf("\nDimensione file compresso = %llu bytes\n", fcompressed_size);

    printf("Dimensione file decompresso = %llu bytes\n", fdecompressed_size);

    // double comp_ratio = (double)fcompressed_size / (double)finput_size;
    // printf("Compression ratio = %f\n", (double)finput_size / (double)fcompressed_size );
    // printf("Saving %f%%\n", (1 - comp_ratio)*100 );

    printf("\nDecompression took %f seconds to execute\n", time_taken);

    printf("%f MB/s\n", fcompressed_size/(time_taken * 1e6));
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

