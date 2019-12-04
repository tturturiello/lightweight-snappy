//
// Created by Timothy Turturiello on 30.11.19.
//

#include <stdio.h>
#include "IO_utils.h"
#include "snappy_decompression.h"
#include "varint.h"

int main() {
    FILE *source;
    FILE *destination;
    char byte_buf;
    char infile_name[] = "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/testWikipediaCompressed";
    char outfile_name[] = "output";
    if((source = fopen(infile_name, "rb"))!= NULL) {
        if ((destination = fopen(outfile_name, "wb"))!= NULL) {
            int file_size = varint_to_dim(source);
            printf("%d", file_size);
            while ((fread(&byte_buf, sizeof(char), 1, source)) != 0) {
                // SNAPPY
                // printf("%X ", readByte);
            }
            fclose(destination);
            fclose(source);
        } else {printf("Errore apertura del file in scrittura");}
    } else {printf("Errore apertura del file in lettura");}
    return 0;
}