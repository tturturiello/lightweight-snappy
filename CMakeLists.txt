cmake_minimum_required(VERSION 3.15)
project(src/snappy C)

set(CMAKE_C_STANDARD 99)

add_executable(SnappyComp src/snappy_compression.c src/IO_utils.c src/varint.c src/BST.c)
add_executable(SnappyDecomp src/snappy_decompression.c src/IO_utils.c src/varint.c)
add_executable(foglio_per_test src/foglio_per_test.c src/IO_utils.c src/BST.c)
add_executable(src/test_varint src/test_varint.c src/varint.c)
add_executable(src/test_BST src/test_BST.c src/BST.c)
add_executable(cmd src/cmd.c src/IO_utils.c src/snappy_compression.c src/snappy_decompression.c src/varint.c src/BST.c src/snappy_compression_tree.c src/result.c src/buffer_compression.c)
add_executable(src/snappy_test src/snappy_test.c src/IO_utils.c src/snappy_compression.c src/snappy_decompression.c src/varint.c src/result.c src/buffer_compression.c)