#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "snappy_compression.h"
#include "snappy_decompression.h"

unsigned int dim[] ={100, 200, 500, 1000, 2000, 5000, 10000, 50000, 100000, 200000, 500000, 1000000, 2000000};

unsigned long long get_file_size(FILE *file) {
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

void create_test_files(FILE *source) {
    char test_name[60];
    char buffer[2000000];

    for (int i = 0; i < 13; ++i) {
        fseek(source, 0, SEEK_SET);//Torno all'inizio del file
        sprintf(test_name, "..\\Standard_test\\%ub5", dim[i]);
        FILE *test = fopen(test_name, "wb");
        assert(test != NULL);

        printf("Letti %u bytes\n", fread(buffer,1, dim[i], source));
        printf("Scrivo %u bytes\n", fwrite(buffer, 1, dim[i], test));
        fclose(test);
    }

}

int main(){

    FILE *source = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\sources_test\\word.docx", "rb");
    assert(source!=NULL);
    create_test_files(source);
    fclose(source);

    char finput_name[300];
    char fcompressed_name[300];
    char fdecompressed_name[300];
    char fcsv_name[300];




/*    FILE *finput;
    FILE *fcompressed;
    FILE *fdecompressed;
    FILE *fcsv;

    for (int i = 0; i < 13; ++i) {
        for (int j = 1; j <= 5; ++j) {

            sprintf(finput_name, "..\\Standard_test\\%ub%d", dim[i], j);
            sprintf(fcompressed_name,
                    "..\\Standard_test\\%ub%d.snp", dim[i], j);

            finput = fopen(finput_name, "rb");
            assert(finput != NULL);
            fcompressed = fopen(fcompressed_name, "wb");
            assert(fcompressed != NULL);

            snappy_compress(finput, get_file_size(finput), fcompressed);

            if (fclose(finput) == 0)
                printf("Chiuso input compressione\n");

            if (fclose(fcompressed) == 0)
                printf("Chiuso output compressione\n");

            if ((fcompressed = fopen(fcompressed_name, "rb")) != NULL) {
                print_result_compression(get_file_size(fcompressed));
            }
            fclose(finput);

            printf("------------------------------------------------------\n\n");
        }
    }
/*
    // unsigned long csv_dim = len(dim)*300;
    // int dim_0 =  *(&dim + 1) - dim;
    //sprintf(fcsv_name, "..\\Standard_test\\%ub%d.csv", csv_dim);

    char dim_input[300];
    char dim_output[300];
    char time[300];
    char mbps[300];
    // fai tre buffer:
        // nomi
        // dim input
        // dim output
        // tempo
        // mb/sec

    fcsv = fopen(fcsv_name, "wb");
    assert(fcsv != NULL);
    int count = 0;
    for (int i = 0; i < 13; ++i) {
        for (int j = 1; j <= 5; ++j) {
            sprintf(finput_name, "..\\Standard_test\\%ub%d.snp", dim[i], j);
            sprintf(fdecompressed_name,
                    "..\\Standard_test\\%ub%d_dec", dim[i], j);

            finput = fopen(finput_name, "rb");
            assert(finput != NULL);
            fdecompressed = fopen(fdecompressed_name, "wb");
            assert(fdecompressed != NULL);

            snappy_decompress(finput, fdecompressed);

            if (fclose(finput) == 0) {
                // printf("Chiuso input compressione\n");
                return -1;
            }

            if (fclose(fdecompressed) == 0) {
                // printf("Chiuso output decompressione\n");
                return -1;
            }

            if ((fdecompressed = fopen(fdecompressed_name, "rb")) != NULL) {
                print_result_decompression(get_file_size(fdecompressed), get_file_size(finput));
            }
            fclose(finput);

            printf("------------------------------------------------------\n\n");
            count++;
        }
    }
    if (fclose(fcsv) == 0)
        printf("Chiuso file csv\n");
        */
}