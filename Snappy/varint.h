#include <stdio.h>


unsigned int parse_to_varint(unsigned int n, unsigned char *varint);
void test_parse_to_varint();

/*
 * ritorna il numero di byte presenti nel file prima della compressione,
 * attraverso la lettura dei primi Byte associati al varint
 */
int varint_to_dim(FILE *source);