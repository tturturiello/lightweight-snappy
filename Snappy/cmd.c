#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "IO_utils.h"
#include "snappy_compression.h"
#include "snappy_compression_tree.h"
#include "snappy_decompression.h"


 static enum {compress, uncompress} mode;
 static FILE *input;
 static FILE *output;



void usage() {
    fprintf(stderr,
            "snappy [-c|-d] [infile] [outfile]\n"
            "-c compressione\n"
            "-d decompressione\n"
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

void show_result(char *output_name) {
    if((output = fopen(output_name, "rb") )!= NULL)  {
        print_result_compression(get_size(output));
    }
    fclose(output);
}

int main(int argc, char* argv[]){

    int opt;

    if(argc < 4){
        usage();
    }
    while ((opt = getopt(argc, argv, "cd")) != -1) {
        switch (opt) {
            case 'c':
                mode = compress;
                printf("compressione\n");
                break;
            case 'd':
                mode = uncompress;
                printf("decompressione\n");
                break;
            default:
                usage();
        }
    }

    char *input_name = argv[2];
    char *output_name = argv[3];

    open_input(input_name);
    open_output(output_name);
    unsigned long long input_size = get_size(input);
    if(mode == compress){
        snappy_compress(input, input_size, output);
    } else if(mode == uncompress) {
        snappy_decompress(input, output);
    }

    fclose(input);
    fclose(output);

    show_result(output_name);

    //TODO compressione non avvenuta?
}



