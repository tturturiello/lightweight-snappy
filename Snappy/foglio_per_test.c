#include <stdio.h>
#include "IO_utils.h"
#include "BST.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

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

void test_load_u32() {
    unsigned char *input = "\x3F""\xEC""\xAD""\x09";

    u32 load;
    load = (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3];
    printf("%X\n", load);
}

void test_tag_literal() {
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

char *write_single_copy(char *output, unsigned int len, unsigned int offset){
    if( (len < 12) && offset < 2048){//Copy 01: 3 bits for len-4 and 11 bits for offset
        *output++ = ((offset >> 8 ) << 5) + ((len - 4) << 2) + 1;
        *output++ = offset & 0xFF;
    } else if ( offset < 65536) {//Copy 10: 6 bits for len-1 and 16 bits for offset //TODO: assert?
        *output++ = ((len - 1)  << 2)  | 2;
        *output++ = offset & 0xFF;
        *output++ = (offset >> 8) & 0xFF;
        //Copy 11 non ? necessaria: il blocco da comprimere ? <= 64kB
    }
    return output;
}

char *test_write_copy(unsigned int len, unsigned int offset, char *output) {
    while(len > 68){ //Garantisco che rimangano un minimo di 4 bytes per utilizzare alla fine la copia 01
        output = write_single_copy(output, 64, offset); //64 ? la max len per una copia
        len-=64;
    }
    if(len > 64) { //64 < len < 68
        output = write_single_copy(output, 60, offset);
        len-=60;
    }

    output = write_single_copy(output, len, offset);
    return output;
}

int main(){

    unsigned int len = 65539;
    unsigned int offset = 1024;

    char *output = (char *)malloc(sizeof(char)*30);
    char *bop = output;

    unsigned int len_minus_1 = len-1;
    if(len_minus_1 < 60) {
        *output++ = (len_minus_1 << 2u) & 0xFF;
    } else {
        char *tag_byte = output++; //Lascio lo spazio per il tag byte
        unsigned int code_literal = 59; //identifica quanti sono i byte utilizzati per codificare len-1

        while(len_minus_1 > 0){
            *output++ = len_minus_1 & 0xFF;
            len_minus_1 = len_minus_1 >> 8;
            code_literal++;
        }
        assert(code_literal >= 60);
        assert(code_literal <= 64);
        *tag_byte = code_literal << 2;
    }

    //memcpy(output)




    puts("\n\nBuffer in output");
    for(int i = 0; bop + i < output; i++){
        printf("%X ", *(bop+i));
    }
}
