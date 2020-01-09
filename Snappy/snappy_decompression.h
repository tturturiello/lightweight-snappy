//
// Created by Timothy Turturiello on 30.11.19.
//

#ifndef SNAPPY_SNAPPY_DECOMPRESSION_H
#define SNAPPY_SNAPPY_DECOMPRESSION_H

/*
typedef union convertion Converter;
typedef struct buffer Buffer;
 */

// typedef struct test Test;

int snappy_decompress(FILE *file_input, FILE *file_decompressed);
void write_result_decompression(unsigned long long fdecompressed_size);

#endif //SNAPPY_SNAPPY_DECOMPRESSION_H