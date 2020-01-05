#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "snappy_compression.h"
#include "snappy_decompression.h"

unsigned int dim[] ={500, 1000, 2000, 5000, 10000, 20000, 50000, 80000, 100000, 200000, 500000, 800000, 1000000};

unsigned long long get_size(FILE *file) {
    unsigned long long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;

/*    struct stat st;//TODO:?

    if (stat(filename, &st) == 0)
        return st.st_size;
    else return -1;*/
}

void compareFiles(FILE *fp1, FILE *fp2) {
    // fetching character of two file
    // in two variable ch1 and ch2
    char ch1 = getc(fp1);
    char ch2 = getc(fp2);

    // error keeps track of number of errors
    // pos keeps track of position of errors
    // line keeps track of error line
    int error = 0, pos = 0, line = 1;

    // iterate loop till end of file
    while (ch1 != EOF && ch2 != EOF) {
        pos++;

        // if both variable encounters new
        // line then line variable is incremented
        // and pos variable is set to 0
        if (ch1 == '\n' && ch2 == '\n') {
            line++;
            pos = 0;
        }

        // if fetched data is not equal then
        // error is incremented
        if (ch1 != ch2) {
            error++;
            printf("Line Number : %d \tError"
                   " Position : %d \n", line, pos);
        }

        // fetching character until end of file
        ch1 = getc(fp1);
        ch2 = getc(fp2);
    }

    printf("Total Errors : %d\t", error);
}

void create_test_files(FILE *source) {
    char test_name[60];
    char buffer[2000000];

    for (int i = 0; i < 13; ++i) {
        fseek(source, 1500, SEEK_SET);//Torno all'inizio del file
        sprintf(test_name, "..\\Standard_test\\%ub5", dim[i]);
        FILE *test = fopen(test_name, "wb");
        assert(test != NULL);

        printf("Letti %u bytes\n", fread(buffer,1, dim[i], source));
        printf("Scrivo %u bytes\n", fwrite(buffer, 1, dim[i], test));
        fclose(test);
    }

}

void run_compression(char *finput_name, char*fcompressed_name){
    FILE *finput;
    FILE *fcompressed;
    printf("\n------------------------------------------------------\n");
    printf("Compressione di %s\n\n", fcompressed_name);
    finput = fopen(finput_name, "rb");
    assert(finput != NULL);
    fcompressed = fopen(fcompressed_name, "wb");
    assert(fcompressed != NULL);

    snappy_compress(finput, get_size(finput), fcompressed);

    if (fclose(finput) == 0)
        printf("Chiuso input compressione\n");

    if (fclose(fcompressed) == 0)
        printf("Chiuso output compressione\n");

    if ((fcompressed = fopen(fcompressed_name, "rb")) != NULL) {
        print_result_compression(get_size(fcompressed));
        write_result_compression(get_size(fcompressed));
    }
    fclose(fcompressed);
    printf("------------------------------------------------------\n\n");

}


int main(){

/*    FILE *source = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\sources_test\\word.docx", "rb");
    assert(source!=NULL);
    create_test_files(source);
    fclose(source);*/

    char finput_name[300];
    char fcompressed_name[300];
    char fdecompressed_name[300];
    FILE *finput;
    FILE *fcompressed;
    FILE *fdecompressed;

    for (int i = 0; i < 13; ++i) {
        for (int j = 1; j <= 5; ++j) {

            sprintf(finput_name, "..\\Standard_test\\%ub%d", dim[i], j);
            sprintf(fcompressed_name,
                    "..\\Standard_test\\%ub%d.snp", dim[i], j);
            sprintf(fdecompressed_name,
                    "..\\Standard_test\\%ub%ddec", dim[i], j);
            
            //-----------------------Compressione----------------------
            run_compression(finput_name, fcompressed_name);


            //-----------------------Decompressione----------------------
            printf("Decompressione di %s\n\n", fdecompressed_name);
            fcompressed = fopen(fcompressed_name, "rb");
            assert(fcompressed != NULL);
            fdecompressed = fopen(fdecompressed_name, "wb");
            assert(fdecompressed != NULL);

            snappy_decompress(fcompressed, fdecompressed);

            if (fclose(fcompressed) == 0)
                printf("Chiuso input decompressione\n");

            if (fclose(fdecompressed) == 0)
                printf("Chiuso output decompressione\n");

            if ((fdecompressed = fopen(fdecompressed_name, "rb")) != NULL) {
                write_result_decompression(get_size(fdecompressed));
            }
            fclose(fdecompressed);

            printf("------------------------------------------------------\n\n");
            printf("CHECK INTEGRITY\n\n");
            finput = fopen(finput_name, "rb");
            assert(finput != NULL);
            fdecompressed = fopen(fdecompressed_name, "rb");
            assert(fdecompressed != NULL);

            compareFiles(finput, fdecompressed);

            fclose(finput);
            fclose(fdecompressed);
        }
    }
}