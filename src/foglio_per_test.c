#include <stdio.h>
#include "IO_utils.h"
#include "BST.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#define MAX_BLOCK_SIZE 65536
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
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


void test_tag_literal(){

    unsigned int len = 360;
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


unsigned int getFileSize(FILE *finput) {
    fseek(finput, 0, SEEK_END);
    int file_size = ftell(finput);
    fseek(finput, 0, SEEK_SET);
    return file_size;
}

int test_blocks() {
    FILE *finput;
    FILE *fout;

    char *input = (char *)calloc(100, sizeof(char));
    char *beginning = input;
    unsigned int n_bytes;

    if((finput = fopen("..\\wikipedia_test.txt", "rb")) == NULL) {
        exit(1);
    }

    if((fout = fopen("..\\output.txt", "wb")) == NULL) {
        exit(1);
    }
    unsigned int file_size = getFileSize(finput);
    n_bytes = min(file_size, MAX_BLOCK_SIZE);

    while(n_bytes > 0){ //TODO empty file
        n_bytes = fread(input, sizeof(char), 100, finput);
        printf("\n\nLetti: %d\n\n", n_bytes);

        printf("\n\nScritti: %d\n\n", fwrite(input, sizeof(char), n_bytes, fout));
        puts(input);
    }

    fclose(finput);
    fclose(fout);
}

void alternative_hash_bytes(u32 val){
    int htable_size = 4096;
    double A =  (sqrt(5.0) - 1 )/ 2;
    double fraction = A * val - ((long)(A * val));
    unsigned int code = (unsigned int)(htable_size * fraction);
    printf("%d",  code);
}

int main() {

    u32 val = 3904133790;
    alternative_hash_bytes(val);

}

