#include <stdio.h>
#include <string.h>
#include "varint.h"

static int error = 0;

void print_varint(unsigned char varint[], unsigned int len) {
    for (int i = 0; i < len; ++i) {
        printf("%X ", varint[i]);
    }
}

void test_parse_varint_with(unsigned int n, unsigned char *expexted_result) {
    unsigned char varint[20];
    unsigned int len;
    len = parse_to_varint(n, varint);
    if(strncmp(varint, expexted_result, len)!=0){
        printf("\nErrore parsing n = %d\n", n);
        error = 1;
    }
}


void test_str_varint_to_dim_with(unsigned int n) {
    unsigned char varint[20];
    parse_to_varint(n, varint);
    if(str_varint_to_dim_(varint)!=n){
        printf("Errore decodifica per n = %d\n", n);
        error = 1;
    }
}

void test_parse_varint() {//Test 127 --> 0x0101
    test_parse_varint_with(127, "\x7F");
    //Test 227 --> 0xE301
    test_parse_varint_with(227, "\xE3\x01");
    //Test 16384 --> 0x808001
    test_parse_varint_with(16384, "\x80\x80\x01");
    puts("Test parse_varint() terminato!");

}

void test_str_varint_to_dim() {
    test_str_varint_to_dim_with(127);
    test_str_varint_to_dim_with(227);
    test_str_varint_to_dim_with(16384);
    puts("Test str_varint_to_dim() terminato!");
}

int main() {
    test_parse_varint();
    test_str_varint_to_dim();

    if(!error)
        puts("Test varint terminato con successo!");
}