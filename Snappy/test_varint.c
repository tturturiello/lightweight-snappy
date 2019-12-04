#include <stdio.h>
#include "varint.h"

void print_varint(unsigned char varint[], unsigned int len) {
    for (int i = 0; i < len; ++i) {
        printf("%X ", varint[i]);
    }
}

void test_parse_varint_with(unsigned int n) {
    unsigned char varint[20];
    unsigned int len;
    len = parse_to_varint(n, varint);
    printf("\n%d in varint: ", n);
    print_varint(varint, len);
}

void test_parse_to_varint(){
    //Test 127 --> 0x0101
    test_parse_varint_with(127);
    //Test 227 --> 0xE301
    test_parse_varint_with(227);
    //Test 227 --> 0xE301
    test_parse_varint_with(16384);
}

int main() {
    test_parse_to_varint();
}