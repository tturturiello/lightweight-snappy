#include <stdio.h>
#include <assert.h>
#include "snappy_compression.h"

#define FINPUT_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Files_test\\wikipedia_test.txt"
#define FCOMPRESSED_NAME "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\Compressed_test\\wikipedia_compressed.snp"
#define FDECOMPRESSED_NAME ""

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
    FILE * finput;
    FILE * fcompressed;
    FILE * fdecompressed;

    finput = fopen(FINPUT_NAME, "rb");
    assert(finput != NULL);
    fcompressed = fopen(FCOMPRESSED_NAME, "wb");
    assert(fcompressed != NULL);

    snappy_compress(finput, get_file_size(finput), fcompressed);

    if(fclose(finput) == 0)
        printf("Chiuso input compressione\n");

    if(fclose(fcompressed) == 0)
        printf("Chiuso output compressione\n");

    if((fcompressed = fopen(FCOMPRESSED_NAME, "rb") )!= NULL) {

        print_result_compression(get_file_size(fcompressed));

    }
    fclose(finput);

}