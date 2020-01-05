#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "snappy_compression.h"
//#include "snappy_compression_tree.h"
#include "snappy_decompression.h"



//------------Belli---------------------------------------------
#define FINPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Standard_test\\5000b4"
#define FCOMPRESSED_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Standard_test\\5000b4.snp"
#define FDECOMPRESSED_NAME "..\\Compressed_test\\alice_decompressed.txt"

/*


//------------Turturiello---------------------------------------------
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Files_test/alice.txt"
#define FCOMPRESSED_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Compressed_test/alice_compressed"
#define FDECOMPRESSED_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Decompressed_test/alice_decompressed.txt"
*/
 static enum {compress, uncompress} mode;
 FILE *input;
 FILE *output;

unsigned long long get_file_size(FILE *file) {
    unsigned long long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;

/*    struct stat st;//TODO:?

    if (stat(filename, &st) == 0)
        return st.st_size;
    else return -1;*/
}

int compression() {

    FILE *finput;
    FILE *fcompressed;


    //------------Compressione ---------------------------------------------
    finput = fopen(FINPUT_NAME, "rb");
    assert(finput != NULL);
    fcompressed = fopen(FCOMPRESSED_NAME, "wb");
    assert(fcompressed != NULL);

    snappy_compress(finput, get_file_size(finput), fcompressed);

    if(fclose(finput) == 0)
        printf("Chiuso input compressione\n");

    if(fclose(fcompressed) == 0)
        printf("Chiuso output compressione\n");

    if((fcompressed = fopen(FCOMPRESSED_NAME, "rb") )!= NULL)  {
        print_result_compression(get_file_size(fcompressed));
    }
    fclose(finput);
}

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
        print_result_compression(get_file_size(output));
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
    unsigned long long input_size = get_file_size(input);
    if(mode == compress){
        snappy_compress(input, input_size, output);
    } else if(mode == uncompress) {
        //snappy_decompress(input, output);
    }

    fclose(input);
    fclose(output);

    show_result(output_name);
}



