#include <stdio.h>

void parse_to_varint(int n, char *varint);

/*
 * ritorna il numero di byte presenti nel file prima della compressione,
 * attraverso la lettura dei primi Byte associati al varint
 */
int varint_to_dim(FILE *source);
void parse_to_varint(int n, char *varint);
void test_parse_to_varint();