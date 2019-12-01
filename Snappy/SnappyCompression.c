#include <stdio.h>
#include "IOUtils.h"
#include "Varint.h"





int main() {
    char readByte;
    FILE *input;
    if((input = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL){

        fseek(input, 0, SEEK_END);
        int file_size = ftell(input);
        fseek(input, 0, SEEK_SET);
        parse_to_varint(file_size);
        printf("%d\n", file_size);
        while( (fread(&readByte, sizeof(char), 1, input))!=0 ){
            printf("%X ", readByte);
        }
    } else {
        puts("Errore apertura file");
    }
}