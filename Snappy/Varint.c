#include <stdio.h>
#include "Varint.h"
const char MSB_mask = 0x80;

void parse_to_varint(int n, char *varint) {
    if (n < 128) {//2^7
        varint[0] = n ;
    } else if (n < 16384) {//2^14

        varint[0] = (char) (n & 0xFF) | MSB_mask;
        varint[1] = (n & MSB_mask)>>7;
    }//TODO: continuare per ogni caso possibile
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

void test_parse_to_varint(){
    char varint[20];
    parse_to_varint(127, varint);
    printf("\n127 in varint: %X\n", varint[0]);
    printf("\n%X\n", ~MSB_mask);
    parse_to_varint(129, varint);
    printf("\n129 in varint:%c\n", varint[0]);
    printf("\n%X\n", varint[1]);
}