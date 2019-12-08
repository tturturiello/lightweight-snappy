#include <stdio.h>
#include <stdlib.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"

static unsigned int htable_size = 14;

static inline u32 hash_bytes(u32 bytes){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) % htable_size;
}


Tree **get_hash_table(int file_size) {
    Tree **hash_table = (Tree **)malloc(sizeof(Tree*)*10);
    return hash_table;
}

int getFileSize(const FILE *input) {
    fseek(input, 0, SEEK_END);
    int file_size = ftell(input);
    fseek(input, 0, SEEK_SET);
    return file_size;
}

void emit_literal(u32 buffer[], int *len) {
    printf("Literal di dimensione %d\n", *len);
    for (int i = 0; i < *len; ++i) {
        printf("%X ", buffer[i]);
    }
    *len = 0;
    printf("\n");

}

int main() {
    u32 literal_buffer[300];
    int literal_counter = 0;
    u32 read_u32;
    FILE *input;


    Tree *hash_table[htable_size];
    for (int i = 0; i < htable_size; ++i) {
        hash_table[i] = create_tree();
    }

    if((input = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL){

        int file_size = getFileSize(input);
        printf("Dimesnione file: %d bytes, %d u32\n", file_size, file_size/4);

        int index = 0;
        Node *copy;
        while( (fread(&read_u32, sizeof(u32), 1, input))!=0 ) {
            index = hash_bytes(read_u32);
            if (is_empty(hash_table[index]) | ((copy = find(read_u32, hash_table[index]) ) == NULL)) {
                //printf("%X literal\n", read_u32);
                literal_buffer[literal_counter++] = read_u32; //Aggiungo il literal in output
                insert(read_u32, ftell(input)/4, hash_table[index]);
            } else {
                emit_literal(literal_buffer, &literal_counter);
                printf("%X copy of %X\n", read_u32, copy->bytes);

            }

        }
        fclose(input);
    } else {
        puts("Errore apertura file");
    }
    puts("\n");
    for (int i = 0; i < htable_size; ++i) {
        print_tree_inorder(hash_table[i]);
        printf("\n");
    }


}