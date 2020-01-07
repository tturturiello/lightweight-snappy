//
// Created by belli on 29.12.2019.
//

#ifndef SNAPPY_SNAPPY_COMPRESSION_TREE_H
#define SNAPPY_SNAPPY_COMPRESSION_TREE_H



int snappy_compress_tree(FILE *file_input, unsigned long long input_size, FILE *file_compressed);
void print_result_compression_tree(unsigned long long fcompressed_size);


#endif //SNAPPY_SNAPPY_COMPRESSION_H
