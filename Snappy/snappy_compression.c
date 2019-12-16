#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"

static unsigned int htable_size = 20;

int find_copy_length(char *input, char *candidate, const char *limit) {//TODO: max copy length? Incremental copy?
    int length = 0;//TODO ottimizzare partendo da +4
    for(;input <= limit; input++, candidate++){
        if(*input==*candidate){
            length++;
        } else {
            return length;
        }
    }
    return length;
}

static inline u32 hash_bytes(u32 bytes){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) % htable_size;
}


Tree **get_hash_table(int file_size) {
    Tree **hash_table = (Tree **)malloc(sizeof(Tree*)*10);
    return hash_table;
}

unsigned int getFileSize(const char *filename) {
/*    fseek(finput, 0, SEEK_END);
    int file_size = ftell(finput);
    fseek(finput, 0, SEEK_SET);
    return file_size;*/
    struct stat st;//TODO:?

    if (stat(filename, &st) == 0)
        return st.st_size;
    else return -1;
}

char *write_literal(const char *input, char *output, unsigned int len) {
    printf("Literal di dimensione %d\n", len);
    //Scrivo tag byte
    if(len < 60){//TODO: generalizzare
        *(output++) = ((len-1) << 2u) & 0xFF;
    } else {
        *(output++) = (60 << 2u);
        *(output++) = (len-1) & 0xFF;
    }

    for (int i = 0; i < len; ++i) {
        *(output++) = input[i];
        printf("%X ", input[i]);
    }
    printf("\n");
    return output;

}

u32 get_next_u32(const unsigned char *input, const unsigned char *limit) {

    return (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3]; //TODO union di conversione
}

char *write_copy(unsigned int length, unsigned long offset, char *output) {
    *(output++) = (length-1) << 2 | 2;//TODO 3 tipi di copia
    *(output++) = offset & 0xFF;
    *(output++) = (offset >> 8u) & 0xFF;
    return output;

}

void write_file_compressed(const char *beginning, char *end) {
    FILE *fcompressed;
    if((fcompressed = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\test_compressed", "w") ) != NULL){
        puts("Inizio scrittura------------\n");
        fwrite(beginning, sizeof(char), end - beginning, fcompressed);
    } else {
        printf("Errore scrittura su file\n");
    }
    fclose(fcompressed);
}

char *write_dim_varint(unsigned int file_dim, char *output) {
    unsigned int size_varint = parse_to_varint(file_dim, output);
    return output + size_varint;
}

int main() {
    int literal_length = 0;
    int copy_length = 0;
    const char *input_file_name = "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt";
    FILE *finput;
    char *input;
    char *output;
    const char * beginning;
    const char * out_beginning;
    char *input_limit;


    Tree *hash_table[htable_size];
    for (int i = 0; i < htable_size; i++) {
        hash_table[i] = create_tree();
    }

    if((finput = fopen(input_file_name, "r") )!= NULL){
        unsigned int file_size = getFileSize(input_file_name);
        input = (char *)calloc(file_size, sizeof(char));
        output = (char *)malloc(sizeof(char)*file_size);
        printf("SCRITTI:%d\n", file_size = fread(input, sizeof(char), file_size, finput) );
        printf("Dimesnione file: %d bytes, %d u32\n", file_size, file_size/4);
        beginning = input;
        out_beginning = output;
        input_limit = input + file_size;

        output = write_dim_varint(file_size, output);

    } else {
        puts("Errore apertura file");
        return 1;
    }
    fclose(finput);

    u32 current_u32 ;
    int index = 0;
    Node *copy;
    char *candidate;
    while( input+4 < input_limit ) {
        current_u32 = get_next_u32(input, input_limit);
        index = hash_bytes(current_u32);
        if (is_empty(hash_table[index]) | ((copy = find(current_u32, hash_table[index]) ) == NULL)) {
            //printf("%X literal\n", current_u32);
            //output[literal_length++] = current_u32; //Aggiungo il literal in output
            insert(current_u32, input - beginning , hash_table[index]);
            literal_length+=4;
            copy_length = 0;
        } else {

            output = write_literal(input - literal_length, output, literal_length);
            literal_length = 0;
            candidate = beginning + copy->offset;
            copy_length = find_copy_length(input+4, candidate + 4, input_limit);
            printf("%X copy of offset = %d and length = %d\n", current_u32, input - candidate, copy_length + 4);
            output = write_copy(copy_length + 4, input - candidate, output);
        }
        input+= 4 + copy_length;
    }
    output = write_literal(input - literal_length, output, literal_length);


    write_file_compressed(out_beginning, output);
    unsigned char byte;

    puts("\n\nFIle originale");
    if((finput = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL) {
        while((fread(&byte, sizeof(char), 1, finput ) !=0) ) {
            printf("%X ", byte);
        }
    }
    fclose(finput);

    puts("\n\nBuffer in input");
    for(int i = 0; beginning + i < input_limit; i++){
        printf("%3d: %X ", i, *(beginning+i));
    }

    puts("\n\nBuffer in output");
    for(int i = 0; out_beginning + i <= output; i++){
        printf("%4d: %X ", i, *(out_beginning+i));
    }

    puts("\n\nFIle compress");
    if((finput = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\test_compressed", "r") )!= NULL){
        while((fread(&byte, sizeof(char), 1, finput ) !=0) ) {
            printf("%X ", byte);

        }
    }

/*    puts("\n");
    for (int i = 0; i < htable_size; ++i) {
        print_tree_inorder(hash_table[i]);
        printf("\n");
    }*/


}


