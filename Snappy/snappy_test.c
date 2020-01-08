#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <winnt.h>
#include <afxres.h>
#include "IO_utils.h"
#include "snappy_compression.h"
#include "snappy_compression_tree.h"
#include "snappy_decompression.h"

unsigned int dim[] ={500, 1000, 2000, 5000, 10000, 20000, 50000, 80000, 100000, 200000, 500000, 800000, 1000000};
static double time_taken = 0;
LARGE_INTEGER frequency;
LARGE_INTEGER start;
LARGE_INTEGER end;
typedef enum {compress, uncompress} Mode;


void compare_files(char *f1_name, char *f2_name) {

    FILE * f1 = open_read(f1_name);
    FILE * f2 = open_read(f2_name);
    // fetching character of two file
    // in two variable ch1 and ch2
    char ch1 = getc(f1);
    char ch2 = getc(f2);

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
            //printf("Line Number : %d \tError"
             //      " Position : %d \n", line, pos);
        }

        // fetching character until end of file
        ch1 = getc(f1);
        ch2 = getc(f2);
    }
    fclose(f1);
    fclose(f2);
    printf("Total Errors : %d\t", error);
    puts("");
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

void start_time() {
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
}

void stop_time() {
    QueryPerformanceCounter(&end);
    time_taken = (double) (end.QuadPart - start.QuadPart) / frequency.QuadPart;
}

void write_result_compression(unsigned long long finput_size, unsigned long long fcompressed_size){
    FILE *csv = open_append("..\\Standard_test\\risultati_compressione.csv");
    fprintf(csv, "%llu, %llu, %f, %f, %f\n",
            finput_size,
            fcompressed_size,
            (double)finput_size / (double)fcompressed_size ,
            time_taken,
            finput_size/(time_taken * 1e6));
    fclose(csv);
}

void write_result_decompressione(unsigned long long finput_size, unsigned long long fdecompressed_size)
{
    FILE *csv = open_append("..\\Standard_test\\risultati_decompressione_1120K.csv");
    fprintf(csv, "%llu, %llu, %f, %f\n",
            finput_size,
            fdecompressed_size,
            time_taken,
            finput_size/(time_taken * 1e6));
    fclose(csv);
}

void run_test(char *finput_name, char*foutput_name, Mode mode){
    FILE *finput;
    FILE *foutput;

    finput = open_read(finput_name);
    foutput = open_write(foutput_name);

    unsigned long long finput_size = get_size(finput);

    start_time();
    if(mode == compress)
        snappy_compress(finput, finput_size, foutput);
    else
        snappy_decompress(finput, foutput);
    stop_time();

    fclose(finput);
    fclose(foutput);


    foutput = open_read(foutput_name);

    unsigned long long foutput_size = get_size(foutput);
    if(mode == compress) {
        //print_result_compression_tree(foutput_size);
        //write_result_compression(finput_size, foutput_size);
    } else {
        //print_result_decompression(foutput_size, finput_size);
        write_result_decompressione(finput_size, foutput_size);
    }

    fclose(foutput);

}



int main(){

/*    FILE *source = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\sources_test\\word.docx", "rb");
    assert(source!=NULL);
    create_test_files(source);
    fclose(source);*/

    char finput_name[300];
    char fcompressed_name[300];
    char fdecompressed_name[300];

    for (int i = 0; i < 13; ++i) {
        for (int j = 1; j <= 5; ++j) {

            sprintf(finput_name, "..\\Standard_test\\%ub%d", dim[i], j);
            sprintf(fcompressed_name,
                    "..\\Standard_test\\%ub%d.snp", dim[i], j);
            sprintf(fdecompressed_name,
                    "..\\Standard_test\\%ub%ddec", dim[i], j);
            //-----------------------Compressione----------------------
            //printf("\n------------------------------------------------------\n");
            printf("Compressione di %s\n\n", fcompressed_name);
            //run_test(finput_name, fcompressed_name, compress);
            for (int k = 0; k < 500; ++k) {


                //-----------------------Decompressione----------------------

                //printf("\n------------------------------------------------------\n");
                //printf("Decompressione di %s\n\n", fcompressed_name);
                run_test(fcompressed_name, fdecompressed_name, uncompress);
            }
            printf("------------------------------------------------------\n\n");
            printf("CHECK INTEGRITY\n\n");

            compare_files(finput_name, fdecompressed_name);


        }
    }
}