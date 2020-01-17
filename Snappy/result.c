#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "result.h"
#include "IO_utils.h"

static double time_taken = 0;
static clock_t t;

void start_time() {
    t = clock();
}

void stop_time() {
    t = clock() - t;
    time_taken = (double) t / CLOCKS_PER_SEC;
}

void write_result_compression(unsigned long long finput_size, unsigned long long fcompressed_size){
    FILE *csv = open_append("..\\Risultati_test\\risultati_compressione.csv");
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
    FILE *csv = open_append("..\\Risultati_test\\risultati_decompressione_1GB.csv");
    fprintf(csv, "%llu, %llu, %f, %f\n",
            finput_size,
            fdecompressed_size,
            time_taken,
            finput_size/(time_taken * 1e6));
    fclose(csv);
}

void write_result_speed(unsigned long long finput_size, unsigned long long fdecompressed_size)
{
    FILE *csv = open_append("..\\Risultati_test\\risultati_speed_decomp_1GB_bufMin.csv");
    fprintf(csv, "%f\n",
            finput_size/(time_taken * 1e6));
    fclose(csv);
}

void print_result_compression(unsigned long long fcompressed_size, unsigned long long int finput_size) {

    printf("\nDimensione file originale = %llu bytes\n", finput_size);

    printf("Dimensione file compresso = %llu bytes\n", fcompressed_size);

    double comp_ratio = (double)fcompressed_size / (double)finput_size;
    printf("Compression ratio = %f\n", (double)finput_size / (double)fcompressed_size );
    printf("Saving %f%%\n", (1 - comp_ratio)*100 );

    printf("\nCompression took %f seconds to execute\n", time_taken);
    printf("%f MB/s\n", finput_size/(time_taken * 1e6));
}

void print_result_decompression(unsigned long long fdecompressed_size, unsigned long long int finput_size) {

    printf("\nDimensione file compresso = %llu bytes\n", finput_size);

    printf("Dimensione file decompresso = %llu bytes\n", fdecompressed_size);

    printf("\nCompression took %f seconds to execute\n", time_taken);
    printf("%f MB/s\n", finput_size/(time_taken * 1e6));
}

void compare_files(char *f1_name, char *f2_name) {

    FILE * f1 = open_read(f1_name);
    FILE * f2 = open_read(f2_name);

    char ch1 = getc(f1);
    char ch2 = getc(f2);

    int error = 0;

    while (ch1 != EOF && ch2 != EOF) {
        if (ch1 != ch2) {
            error++;
        }
        ch1 = getc(f1);
        ch2 = getc(f2);
    }
    fclose(f1);
    fclose(f2);
    printf("Total Errors : %d\n", error);

    assert(error == 0);
}
