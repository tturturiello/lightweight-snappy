#ifndef SNAPPY_VARINT_H
#define SNAPPY_VARINT_H

unsigned int parse_to_varint(unsigned long long n, unsigned char *varint);

int varint_to_dim(FILE *source);

#endif