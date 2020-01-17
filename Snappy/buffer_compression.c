#include <stdlib.h>
#include "buffer_compression.h"

/**
 * Inizializza il buffer allocando dinamicamente un array di byte di
 * dimensione buffer_size.
 * @param bf il buffer da inizializzare
 * @param buffer_size la dimensione del buffer
 */
void init_Buffer(Buffer *bf, unsigned int buffer_size){
    bf->current = (char *)calloc(buffer_size, sizeof(char));
    bf->beginning = bf->current;
    bf->bytes_left = buffer_size;
}

/**
 * Sposta la posizione del puntatore current dell'offset specificato
 * nel buffer bf
 * @param bf il buffer da modificare
 * @param offset
 */
void move_current(Buffer *bf, unsigned int offset){
    bf->current += offset;
    bf->bytes_left -= offset;
}

/**
 * Assegna la posizione di current a quella di beginning resettando cos?
 * il buffer
 * @param bf il buffer da resettare
 */
void reset(Buffer *bf) {
    bf->current = bf->beginning;
}
