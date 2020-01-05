#include <stdio.h>
#include <assert.h>
#include <getopt.h>
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
    FILE *fdecompressed;


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

void show_usage() {

}

int main(int argc, char* argv[]){
    int opt;
    while ((opt = getopt(argc, argv, "cd")) != -1) {
        switch (opt) {
            case 'c':
                mode = uncompress;
                break;
            case 'd':
                mode = compress;
                break;
            default:
                show_usage();
        }
    }


    //-----------Decompressione ---------------------------------------------
/*
    fcompressed = fopen(FCOMPRESSED_NAME, "rb");
    assert(fcompressed != NULL);
    fdecompressed = fopen(FDECOMPRESSED_NAME, "wb");
    assert(fdecompressed != NULL);

    snappy_decompress(fcompressed, fdecompressed);

    if(fclose(fcompressed) == 0)
        printf("Chiuso input decompressione\n");

    if(fclose(fdecompressed) == 0)
        printf("Chiuso output decompressione\n");

    // if((fdecompressed = fopen(FCOMPRESSED_NAME, "rb") )!= NULL)  {
    //     print_result_decompression(get_file_size(fdecompressed));
    // }

    fclose(fdecompressed);
*/

}



