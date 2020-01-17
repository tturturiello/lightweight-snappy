#include <stdio.h>
#include "varint.h"
const char MSB_mask = 0x80;

/**
 * Scrive il numero specificato in formato varint. La sequenza di byte generata viene scritta
 * a partire dal puntatore a carattere specificato.
 * @param n il numero da scrivere in formato varint
 * @param varint
 * @return il numero di byte utilizzati per la codifica in varint
 */
unsigned int parse_to_varint(unsigned long long n, unsigned char *varint) {
    unsigned char *start = varint;
    while(n & MSB_mask){
        *(varint++) = (n & 0xFF) | MSB_mask;
        n = n >> 7u;
    }
    *varint = n;
    return varint - start + 1;
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
    } while ((byte_buf & mask_condition)!=0);
    return result;
}

int str_varint_to_dim_(unsigned char *varint)
{
    unsigned char mask_condition = 0x80;
    unsigned char mask_value = 0x7f; // ~mask_condition
    unsigned char byte_buf;
    int multiplier = 1;
    int result = 0;

    do {
        byte_buf = *(varint++);
        result += (((int)(byte_buf&mask_value)) * multiplier);
        multiplier *= 128; // equivale a shiftare di 7 bit
    } while ((byte_buf & mask_condition)!=0);
    return result;
}

int str_varint_to_dim_mark(unsigned char *varint, unsigned long *mark)
{
    unsigned char mask_condition = 0x80;
    unsigned char mask_value = 0x7f; // ~mask_condition
    unsigned char byte_buf;
    int multiplier = 1;
    int result = 0;

    do {
        byte_buf = *(varint++);
        result += (((int)(byte_buf&mask_value)) * multiplier);
        multiplier *= 128; // equivale a shiftare di 7 bit
        (*mark)++;
    } while ((byte_buf & mask_condition)!=0);
    return result;
}