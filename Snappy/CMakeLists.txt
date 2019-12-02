cmake_minimum_required(VERSION 3.15)
project(Snappy C)

set(CMAKE_C_STANDARD 99)

add_executable(SnappyComp SnappyCompression.c IOUtils.c Varint.c)
add_executable(SnappyDecomp SnappyDecompression.c IOUtils.c Varint.c)
add_executable(foglio_per_test foglio_per_test.c IOUtils.c)