#include <winnt.h>
#include <afxres.h>
#include <stdio.h>
#include "result.h"
#include "IO_utils.h"

static double time_taken = 0;
LARGE_INTEGER frequency;
LARGE_INTEGER start;
LARGE_INTEGER end;


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

void print_result_compression(unsigned long long fcompressed_size, unsigned long long int finput_size) {

    printf("\nDimensione file originale = %llu bytes\n", finput_size);

    printf("Dimensione file compresso = %llu bytes\n", fcompressed_size);

    double comp_ratio = (double)fcompressed_size / (double)finput_size;
    printf("Compression ratio = %f\n", (double)finput_size / (double)fcompressed_size );
    printf("Saving %f%%\n", (1 - comp_ratio)*100 );

/*    printf("\nNumero di u32 processati = %llu\n", number_of_u32 );
    printf("Numero di collisioni = %llu\n", collisions );
    printf("In percentuale: %f%%\n", ((double)collisions / (double)number_of_u32)*100 );*///TODO togliere collision!

    printf("\nCompression took %f seconds to execute\n", time_taken);
    printf("%f MB/s\n", finput_size/(time_taken * 1e6));
}

//TODO funzione presa da Geek..
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
