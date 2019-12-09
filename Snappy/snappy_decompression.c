//
// Created by Timothy Turturiello on 30.11.19.
//

#include <stdio.h>
#include "IO_utils.h"
#include "snappy_decompression.h"
#include "varint.h"
void do_copy(unsigned char* byte_buf, short num_bytes)
{

}

void do_literal(FILE *source)
{

}

// ritorna l'offset di lettura del supp_buff
//unsigned long decompressor(char *supp_buf, FILE* source)
unsigned long decompressor(char* supp_buf, FILE* source)
{
    unsigned char byte_buf;
    fread(&byte_buf, sizeof(char), 1, source); // inserisce il prossimo byte nel byte_buf
    unsigned char mask_tag = 0x03;
    unsigned char mask_notag = ~mask_tag;
    unsigned char mask_dx_notag = 0x1C; // per la copia di tag 01
    unsigned char mask_sx_notag = 0xE0; // per la copia di tag 01
    unsigned char notag_value = (mask_notag&byte_buf)>>2;
    unsigned char assoc_bytes = 0; // numero di byte associati
    unsigned int offset = 0;
    unsigned int temp_offset = 0;

    unsigned long len = 0;

    //fread(&byte_buf, sizeof(char),1, source);

    unsigned char mode = mask_tag&byte_buf;
    switch(mode) {
        case 0: // do_literal()
            switch (notag_value) {
                case 63:
                    assoc_bytes++;
                case 62:
                    assoc_bytes++;
                case 61:
                    assoc_bytes++;
                case 60:
                    assoc_bytes++;

                    fread(&len, sizeof(char)*assoc_bytes, 1, source);
                    // TODO: riempire supp_buf dal puntatore attuale
                    len++;
                    fread(supp_buf, sizeof(char) * len, 1, source);
                    break;
                default: // <60 len = val+1
                    len = notag_value+1;
                    fread(supp_buf, sizeof(char), len, source);
            }
            // fwrite(&byte_buf, sizeof(char)*assoc_bytes, 4, source); // NO
            // avendo assoc_bytes, leggi tot bytes associati
            break;
            // copie
        case 1:
            // similmente al literal ho al massimo 4 byte da riempire con il valore dell'offset
            // shift 2 per non considerare il tag
            len = ((byte_buf&mask_dx_notag)>>2)+4;
            temp_offset = byte_buf&mask_sx_notag;
            temp_offset = temp_offset>>2;

            // shifo per portare le tre cifre alle più significative del byte da leggere come offset
            temp_offset = temp_offset << 5;
            fread(&offset, sizeof(char), 1, source);
            offset+=temp_offset;
            fseek(source, -2*sizeof(char), 1);  // mi riporto prima dell'offset letto e all'inizio del byte

            // leggo a partire dall'offset
            fseek(source, -(offset * sizeof(char)), 1);
            fread(supp_buf, sizeof(char) * len, 1, source);
            fseek(source, sizeof(char)*offset+1, 1); // rewind alla posizione attuale
            break;
        case 2:
            // prossimi 2 byte
            fread(&offset, sizeof(char)*2, 1, source);
            offset+=temp_offset;
            fseek(source, -3*sizeof(char), 1);  // mi riporto prima dell'offset letto e all'inizio del byte

            // leggo a partire dall'offset
            fseek(source, -(offset * sizeof(char)), 1);
            fread(supp_buf, sizeof(char) * len, 1, source);
            fseek(source, sizeof(char)*offset+1, 1); // rewind alla posizione attuale
            break;
        case 3:
            // prossimi 4 byte
            fread(&offset, sizeof(char)*4, 1, source);
            offset+=temp_offset;
            fseek(source, -5*sizeof(char), 1);  // mi riporto prima dell'offset letto e all'inizio del byte

            // leggo a partire dall'offset
            fseek(source, -(offset * sizeof(char)), 1);
            fread(supp_buf, sizeof(char) * len, 1, source);
            fseek(source, sizeof(char)*offset+1, 1); // rewind alla posizione attuale
            break;
        default:break;
    }
    return len;
}

/// riempie un array (container) da un indice di partenza, con il contenuto di un altro array
/// di dimensione pari o minore al primo (supplier).
/// \param container
/// \param supplier
/// \param pointer
void fill_in_buff(char container[], char supplier[], unsigned int index, unsigned int length)
{
    for (int i = 0; i < length; ++i) {
        container[index+i] = supplier[i];
    }
}


int main() {
    // "/Users/T/Desktop/Git_SNAPPY/asd20192020tpg3/Snappy/testWikipediaCompressed";

    FILE *source;
    FILE *destination;
    char infile_name[] = "testWikipediaCompressed";
    char outfile_name[] = "output.txt";
    unsigned long dimension;
    unsigned long buffer_dim = 65536;

    if ((source = fopen(infile_name, "rb"))== NULL) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
    if ((destination = fopen(outfile_name, "wb")) == NULL ) {
        printf("Errore apertura del file in lettura");
        return -1;
    }
    dimension = varint_to_dim(source);
    do_literal(source); // dato che non ci sono ancora copie, mi aspetto un literal

    char pack_buf[buffer_dim];
    int out = 0;
    unsigned long read_past = 0;
    unsigned long read_pres = 0;
    unsigned long read_total = 0;
    while (out<2) {
        //TODO: Assicurarsi di coprire queste situazioni
        // può accadere che
        //   -   il pacchetto è pieno prima del completamento della trascrizione di copia o literal (fare in modo che il pacchetto copra la dimensione del literal)
        //   -   il buffer
        char supp_pack_buff[buffer_dim];
        read_pres = decompressor(supp_pack_buff, source);
        read_total+=read_pres;
        fill_in_buff(pack_buf, supp_pack_buff, read_past, read_pres);
        read_past = read_pres;
        out++;
    }
    // scrivo su file
    fwrite(pack_buf, sizeof(char),read_total,destination);
    fclose(destination);
    fclose(source);
    return 0;




    return 0;
}