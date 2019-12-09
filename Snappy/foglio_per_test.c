#include <stdio.h>
#include "IO_utils.h"
#include "BST.h"
#include <stdlib.h>
#include <time.h>

void prova_con_BTS(){
    srand(time(NULL));
    Tree *hash_table[10];
    for (int i = 0; i < 10; ++i) {
        hash_table[i] = create_tree();
        printf("%d\n", is_empty(hash_table[i]));
        insert(rand(), 0, hash_table[i]);
        insert(rand(), 0, hash_table[i]);
        insert(rand(), 0, hash_table[i]);
    }

    for (int i = 0; i < 10; ++i) {
        print_tree_inorder(hash_table[i]);
        printf("\n");
    }
}

int main(){
    unsigned char *input = "\x3F""\xEC""\xAD""\x09";

    u32 load;
    load = (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3];
    printf("%X\n", load);
    unsigned int len = 67;
    unsigned char output;
    if(len < 60){//TODO: generalizzare
        output = ((len-1) << 2u) & 0xFF;
        printf("%X", output);
    } else {
        output = 60 << 2u;
        printf("%X ", output);
        output = (len-1) & 0xFF;
        printf("%X", output);

    }
    puts("");
    len = 7;
    unsigned long offset = 63;
    output = (len-1) << 2 | 2;//TODO 3 tipi di copia
    printf("%X ", output);

    output = offset & 0xFF;
    printf("%X ", output);

    output = (offset >> 8u) & 0xFF;
    printf("%X ", output);






}
