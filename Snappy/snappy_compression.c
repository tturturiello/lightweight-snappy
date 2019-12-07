#include <stdio.h>
#include <stdlib.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"


static inline u32 hash_bytes(u32 bytes, unsigned int htable_size){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) % 10;
}


Tree **get_hash_table(int file_size) {
    Tree **hash_table = (Tree **)malloc(sizeof(Tree*)*10);
    return hash_table;
}

int main() {
    u32 read_u32;
    FILE *input;
    Tree *hash_table[10];
    for (int i = 0; i < 10; ++i) {
        hash_table[i] = create_tree();
    }

    if((input = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL){

        fseek(input, 0, SEEK_END);
        int file_size = ftell(input);
        fseek(input, 0, SEEK_SET);
        printf("Dimesnione file: %d bytes, %d u32\n", file_size, file_size/4);



        int index = 0;
        while( (fread(&read_u32, sizeof(u32), 1, input))!=0 ){
            index = hash_bytes(read_u32, 10);
            insert(read_u32, hash_table[index]);

        }
        fclose(input);
    } else {
        puts("Errore apertura file");
    }
    for (int i = 0; i < 10; ++i) {
        print_tree_inorder(hash_table[i]);
        printf("\n");
    }
}