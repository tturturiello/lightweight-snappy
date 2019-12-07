#ifndef SNAPPY_VARINT_H
#define SNAPPY_VARINT_H

unsigned int parse_to_varint(unsigned int n, unsigned char *varint);

/*
 * ritorna il numero di byte presenti nel file prima della compressione,
 * attraverso la lettura dei primi Byte associati al varint
 */
int varint_to_dim(FILE *source);

int str_varint_to_dim_(unsigned char *varint);

#endif