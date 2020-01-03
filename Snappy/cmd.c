#include <stdio.h>
#include <assert.h>
#include "snappy_compression.h"
#include "snappy_decompression.h"


/*
//------------Belli---------------------------------------------
#define FINPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Files_test\\alice.txt"
#define FCOMPRESSED_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Compressed_test\\alice_compressed.snp"
#define FDECOMPRESSED_NAME "..\\Compressed_test\\alice_decompressed.txt"
*/

//------------Turturiello---------------------------------------------
#define FINPUT_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Files_test/alice.txt"
#define FCOMPRESSED_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Compressed_test/alice_compressed"
#define FDECOMPRESSED_NAME "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/Decompressed_test/alice_decompressed.txt"

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

int main(){
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

/*
    //-----------Decompressione ---------------------------------------------
    fcompressed = fopen(FCOMPRESSED_NAME, "rb");
    assert(fcompressed != NULL);
    fdecompressed = fopen(FDECOMPRESSED_NAME, "wb");
    assert(fdecompressed != NULL);

    snappy_decompress(fcompressed, fdecompressed);

    if(fclose(fcompressed) == 0)
        printf("Chiuso input decompressione\n");

    if(fclose(fdecompressed) == 0)
        printf("Chiuso output decompressione\n");
*/
}