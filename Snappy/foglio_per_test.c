#include <stdio.h>
#include "IOUtils.h"

int main(){
    int n = 227;
    unsigned char c = n & 0xFF;
    printf("\n%X\n", c);
    c|=(char)0x80;
    //print_char_as_bits(c);
    printf("\n%X", c);

}
