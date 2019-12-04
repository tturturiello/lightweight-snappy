#include <stdio.h>
#include "IO_utils.h"
#include "varint.h"





int main() {
    char readByte;
    FILE *input;
    if((input = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL){

        fseek(input, 0, SEEK_END);
        int file_size = ftell(input);
        fseek(input, 0, SEEK_SET);

        printf("%d\n", file_size);
        while( (fread(&readByte, sizeof(char), 1, input))!=0 ){
            printf("%X ", readByte);
        }
    } else {
        puts("Errore apertura file");
    }

    test_parse_to_varint();
}