//
// Created by Timothy Turturiello on 30.11.19.
//

#include <stdio.h>
#include "SnappyDecompression.h"


// ritorna il numero di byte presenti nel file prima della compressione,
// attraverso la lettura dei primi Byte associati al varint
int file_dimension(FILE *source)
{
    unsigned char mask_condition = 0x80;
    unsigned char mask_value = 0x7f; // ~mask_condition
    unsigned char byte_buf;
    int multiplier = 1;
    int result = 0;

    do {
        fread(&byte_buf, sizeof(char),1, source); // aggiorna il buffer e sposta il puntatore nel file
        result += (((int)(byte_buf&mask_value)) * multiplier);
        multiplier *= 128; // equivale a shiftare di 7 bit
    } while ((byte_buf&mask_condition)!=0);
    return result;
}

int main() {
    // readFile("testWikipediaCompressed");
    FILE *source;
    FILE *destination;
    char readByte;
    char infile_name[] = "testWikipediaCompressed";
    char outfile_name[] = "output";
    if((source = fopen(infile_name, "rb"))!= NULL) {
        if ((destination = fopen(outfile_name, "wb"))!= NULL) {
            int dimension = file_dimension(source);
            printf("%d", dimension);
            while ((fread(&readByte, sizeof(char), 1, source)) != 0) {
                // SNAPPY
                // printf("%X ", readByte);
            }
            fclose(destination);
            fclose(source);
        } else {printf("Errore apertura del file in scrittura");}
    } else {printf("Errore apertura del file in lettura");}
    return 0;
}