//
// Created by belli on 17.01.2020.
//

#ifndef SNAPPY_BUFFER_COMPRESSION_H
#define SNAPPY_BUFFER_COMPRESSION_H

typedef struct buffer {
    char *current;
    char *beginning;
    unsigned int bytes_left;
} Buffer;

void init_Buffer(Buffer *bf, unsigned int buffer_size);
void move_current(Buffer *bf, unsigned int offset);
void reset(Buffer *bf) ;

#endif //SNAPPY_BUFFER_COMPRESSION_H
