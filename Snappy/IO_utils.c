#include <stdio.h>
#include <assert.h>
#include "IO_utils.h"


void print_char_as_bits(char c)
{
    int mask = 1;
    mask = mask << 7; //da ordine inferiore a ordine superiore!
    for (int i = 1; i <= 8; i++) {
        //verifica il bit più a sinistra di value!
        putchar(((c & mask) == 0) ? '0' : '1');
        c = c << 1;
    }
    putchar('\n');
}

FILE *open_read(char *file_name){
    FILE *file = fopen(file_name, "rb");
    assert(file != NULL);
    return file;
}

FILE *open_write(char *file_name){
    FILE *file = fopen(file_name, "wb");
    assert(file != NULL);
    return file;
}

FILE *open_append(char *file_name){
    FILE *file = fopen(file_name, "a");
    assert(file != NULL);
    return file;
}

unsigned long long get_size(FILE *file) {
    unsigned long long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;

/*    struct stat st;//TODO:?

    if (stat(filename, &st) == 0)
        return st.st_size;
    else return -1;*/
}