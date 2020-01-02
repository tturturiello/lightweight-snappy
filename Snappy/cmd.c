#include <stdio.h>
#include <assert.h>
#include "snappy_compression.h"

#define FINPUT_NAME ""
#define FCOMPRESSED_NAME ""
#define FDECOMPRESSED_NAME ""

int main(){
    FILE * finput;
    FILE * fcompressed;
    FILE * fdecompressed;

    finput = fopen(FINPUT_NAME, "wb");
    assert(finput != NULL);
    fcompressed = fopen(FCOMPRESSED_NAME, "wb");
    assert(fcompressed != NULL);

    snappy_compress(finput, fcompressed);

    if(fclose(finput) == 0)
        printf("Chiuso input compressione\n");

    if(fclose(fcompressed) == 0)
        printf("Chiuso output compressione\n");

}