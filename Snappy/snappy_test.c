#include <stdio.h>
#include "IO_utils.h"
#include "snappy_compression.h"
#include "snappy_decompression.h"
#include "result.h"

unsigned int dim[] ={500, 1000, 2000, 5000, 10000, 20000, 50000, 80000, 100000, 200000, 500000, 800000, 1000000};
char *file_test_name[30] = { "32k_ff",
                               "32k_random",
                               "alice.txt",
                               "empty",
                               "ff_ff_ff",
                               "immagine.tiff"};



typedef enum {compress, uncompress} Mode;

/**
 * Chiama compressione o decompressione in mase al parametro mode, utilizzando i nomi dei file specificati,
 * e stampa i risultati a terminale
 * @param finput_name
 * @param foutput_name
 * @param mode
 */
void run_test_mode(char *finput_name, char*foutput_name, Mode mode){
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
        print_result_compression(foutput_size, finput_size);
    } else {
        print_result_decompression(foutput_size, finput_size);
    }

    fclose(foutput);

}

/**
 * Chiama in successione la compressione, la decompressione e un test di integrità utilizzando
 * i nomi dei file specificati
 * @param finput_name
 * @param fcompressed_name
 * @param fdecompressed_name
 */
void run_test(char *finput_name, char *fcompressed_name, char *fdecompressed_name) {
    printf("\n------------------------------------------------------\n");
    printf("Compressione di %s\n\n", finput_name);
    run_test_mode(finput_name, fcompressed_name, compress);
    printf("\n------------------------------------------------------\n");
    printf("Decompressione di %s\n\n", fcompressed_name);
    run_test_mode(fcompressed_name, fdecompressed_name, uncompress);
    printf("------------------------------------------------------\n\n");
    printf("CHECK INTEGRITY\n\n");
    compare_files(finput_name, fdecompressed_name);
}

int main(){

    char finput_name[300];
    char fcompressed_name[300];
    char fdecompressed_name[300];

    for (int i = 0; i < 6; ++i) {
        sprintf(finput_name, "../Files_test/%s", file_test_name[i]);
        sprintf(fcompressed_name, "../Compressed_test/%s_snp", file_test_name[i]);
        sprintf(fdecompressed_name, "../Decompressed_test/%s_dec", file_test_name[i]);

        run_test(finput_name, fcompressed_name, fdecompressed_name);
    }


    for (int i = 0; i < 13; ++i) {
        for (int j = 1; j <= 5; ++j) {

            sprintf(finput_name, "../Files_test/%ub%d", dim[i], j);
            sprintf(fcompressed_name,
                    "../Compressed_test/%ub%d.snp", dim[i], j);
            sprintf(fdecompressed_name,
                    "../Decompressed_test/%ub%ddec", dim[i], j);

            run_test(finput_name, fcompressed_name, fdecompressed_name);
        }
    }
}


