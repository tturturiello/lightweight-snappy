#include <stdio.h>
#include "varint.h"
const char MSB_mask = 0x80;


unsigned int parse_to_varint(unsigned int n, unsigned char *varint) {
    unsigned char *start = varint;
    while(n & 0x7F){
        *(varint++) = (n & 0xFF) | MSB_mask;
        n = n >> 7u;
    }
    *varint = n;
    return varint - start;
}


int varint_to_dim(FILE *source)
{
    unsigned char mask_condition = 0x80;
    unsigned char mask_value = 0x7f; // ~mask_condition
    unsigned char byte_buf;
    int multiplier = 1;
    int result = 0;

    do {
        fread(&byte_buf, sizeof(char),1, source); // aggiorna il buffer e sposta il puntatore del al prossimo byte
        result += (((int)(byte_buf&mask_value)) * multiplier);
        multiplier *= 128; // equivale a shiftare di 7 bit
    } while ((byte_buf&mask_condition)!=0);
    return result;
}

