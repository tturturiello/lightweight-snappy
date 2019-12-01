#include "Varint.h"

void parse_to_varint(int n, char *varint) {
    if (n < 128) {//2^7
        varint[0] = n;
    } else if (n < 16384) {//2^14

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