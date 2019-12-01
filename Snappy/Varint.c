#include "Varint.h"

void parse_to_varint(int n, char *varint) {
    if (n < 128) {//2^7
        varint[0] = n;
    } else if (n < 16384) {//2^14

    }//TODO: continuare per ogni caso possibile
}