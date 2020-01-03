cmake_minimum_required(VERSION 3.15)
project(Snappy C)

set(CMAKE_C_STANDARD 99)

add_executable(SnappyComp snappy_compression.c IO_utils.c varint.c BST.c)
add_executable(SnappyDecomp snappy_decompression.c IO_utils.c varint.c)
add_executable(foglio_per_test foglio_per_test.c IO_utils.c BST.c)
add_executable(test_varint test_varint.c varint.c)
add_executable(test_BST test_BST.c BST.c)
add_executable(cmd cmd.c snappy_compression_tree.c snappy_decompression.c varint.c BST.c)
add_executable(snappy_test snappy_test.c snappy_compression.c varint.c)