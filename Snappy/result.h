#ifndef SNAPPY_RESULT_H
#define SNAPPY_RESULT_H
void start_time();

void stop_time();

void write_result_compression(unsigned long long finput_size, unsigned long long fcompressed_size);

void write_result_decompressione(unsigned long long finput_size, unsigned long long fdecompressed_size);

void compare_files(char *f1_name, char *f2_name);

#endif //SNAPPY_RESULT_H
