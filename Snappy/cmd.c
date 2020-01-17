#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "IO_utils.h"
#include "snappy_compression.h"
#include "snappy_compression_tree.h"
#include "snappy_decompression.h"
#include "result.h"


static enum {compress, compress_bst, uncompress} mode;
 static FILE *input;
 static FILE *output;



void usage() {
    fprintf(stderr,
            "snappy [-c|-d|-b] [-r] [infile] [outfile]\n"
            "-c compressione\n"
            "-b compressione con hash table di BST\n"
            "-d decompressione\n"
            "-r mostra risultati sul terminale\n"
            "Comprimi o decomprimi un file con snappy\n");
    exit(EXIT_FAILURE);
}

void open_input(char *input_name) {
    if((input = fopen(input_name, "rb") ) == NULL){
        fprintf(stderr, "Errore apertura %s: %s\n", input_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void open_output(char *output_name) {
    if((output = fopen(output_name, "wb") ) == NULL){
        fprintf(stderr, "Errore apertura %s: %s\n", output_name, strerror(errno));
        fclose(input);
        exit(EXIT_FAILURE);
    }
}

void show_result(char *output_name, unsigned long long int finput_size) {
    output = open_read(output_name);
    if(mode == uncompress)
        print_result_decompression(get_size(output), finput_size);
    else
        print_result_compression(get_size(output), finput_size);


    fclose(output);
}

int main(int argc, char* argv[]){

    int print_result = 0;
    int opt;

    if(argc < 4){
        usage();
    }
    while ((opt = getopt(argc, argv, "cbdr")) != -1) {
        switch (opt) {
            case 'c':
                mode = compress;
                printf("compressione\n");
                break;
            case 'b':
                mode = compress_bst;
                printf("compressione bst\n");
                break;
            case 'd':
                mode = uncompress;
                printf("decompressione\n");
                break;
            case 'r':
                print_result = 1;
                printf("Mostro risultati\n");
                break;
            default:
                usage();
        }
    }

    char *input_name = argv[argc - 2];
    char *output_name = argv[argc - 1];

    open_input(input_name);
    open_output(output_name);
    unsigned long long input_size = get_size(input);
    //TODO file larger 4GB? get_size = 0?
    start_time();
    if(mode == compress){
        snappy_compress(input, input_size, output);
    } else if(mode == compress_bst) {
        snappy_compress_bst(input, input_size, output);
    } else if(mode == uncompress) {
        snappy_decompress(input, output);
    }
    stop_time();
    
    fclose(input);
    fclose(output);

    if(print_result)
        show_result(output_name, input_size);
}



