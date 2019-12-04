#include <stdio.h>
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