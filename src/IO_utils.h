#ifndef SNAPPY_IO_UTILS_H
#define SNAPPY_IO_UTILS_H

void print_char_as_bits(char c);
FILE *open_read(char *file_name);
FILE *open_write(char *file_name);
FILE *open_append(char *file_name);
unsigned long long get_size(FILE *file);

#endif