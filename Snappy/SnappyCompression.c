#include <stdio.h>
#include <stdint-gcc.h>
#include "IOUtils.h"



int main() {
    char readByte;
    FILE *input;
    if((input = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL){
        while( (fread(&readByte, sizeof(char), 1, input))!=0 ){
            printf("%X ", readByte);
            printf("%c ", readByte);
        }
    } else {
        puts("Errore lettura file");
    }
}