#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "IO_utils.h"
#include "varint.h"
#include "BST.h"

#define MAX_BLOCK_SIZE 65536
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

static unsigned int htable_size = 20;

unsigned int find_copy_length(char *input, char *candidate, const char *limit) {//TODO: max copy length? Incremental copy?
    unsigned int length = 0;
    for(;input <= limit; input++, candidate++){
        if(*input==*candidate){
            length++;
        } else {
            return length;
        }
    }
    return length;
}

static inline u32 hash_bytes(u32 bytes){
    u32 kmul = 0x1e35a7bd;
    return (bytes * kmul) % htable_size;
}


Tree **get_hash_table(int file_size) {
    Tree **hash_table = (Tree **)malloc(sizeof(Tree*)*10);
    return hash_table;
}



char *write_literal(const char *input, char *output, unsigned int len) {
    printf("Literal di dimensione %d\n", len);
    unsigned int len_minus_1 = len-1;
    if(len_minus_1 < 60) {
        *output++ = (len_minus_1 << 2u) & 0xFF;
    } else {
        char *tag_byte = output++; //Lascio lo spazio per il tag byte
        unsigned int code_literal = 59; //identifica quanti sono i byte utilizzati per codificare len-1

        while(len_minus_1 > 0){
            *output++ = len_minus_1 & 0xFF;
            len_minus_1 = len_minus_1 >> 8;
            code_literal++;
        }
        assert(code_literal >= 60);
        assert(code_literal <= 64);
        *tag_byte = code_literal << 2;
    }

    for (int i = 0; i < len; ++i) {//TODO memcpy()?
        *output++ = input[i];
        printf("%X ", input[i]);
    }
    printf("\n");
    return output;

}



char *write_single_copy(char *output, unsigned int len, unsigned int offset){
    if( (len < 12) && offset < 2048){//Copy 01: 3 bits for len-4 and 11 bits for offset
        *output++ = ((offset >> 8 ) << 5) + ((len - 4) << 2) + 1;
        *output++ = offset & 0xFF;
    } else if ( offset < 65536) {//Copy 10: 6 bits for len-1 and 16 bits for offset //TODO: assert?
        *output++ = ((len - 1)  << 2)  | 2;
        *output++ = offset & 0xFF;
        *output++ = (offset >> 8) & 0xFF;
        //Copy 11 non ? necessaria: il blocco da comprimere ? <= 64kB
    }
    return output;
}

char *write_copy(char *output, unsigned int len, unsigned long offset) {

    while(len > 68){ //Garantisco che rimangano un minimo di 4 bytes per utilizzare alla fine la copia 01
        output = write_single_copy(output, 64, offset); //64 ? la max len per una copia
        len-=64;
    }
    if(len > 64) { //64 < len < 68
        output = write_single_copy(output, 60, offset);
        len-=60;
    }

    output = write_single_copy(output, len, offset);
    return output;

}


/*char *write_dim_varint(unsigned int file_dim, char *output) {
    unsigned int size_varint = parse_to_varint(file_dim, output);
    return output + size_varint;
}*/

void print_result_compression(FILE *finput, const char *output, const char *beginning, const char *out_beginning,
                              const char *input_limit) {
    unsigned char byte;

    puts("\n\nFIle originale");
    if((finput = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt", "r") )!= NULL) {
        while((fread(&byte, sizeof(char), 1, finput ) !=0) ) {
            printf("%X ", byte);
        }
    }
    fclose(finput);

    puts("\n\nBuffer in input");
    for(int i = 0; beginning + i < input_limit; i++){
        printf("%3d: %X ", i, *(beginning+i));
    }

    puts("\n\nBuffer in output");
    for(int i = 0; out_beginning + i <= output; i++){
        printf("%X ", *(out_beginning+i));
    }

    puts("\n\nFIle compress");
    if((finput = fopen("C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\test_compressed", "rb") )!= NULL){
        while((fread(&byte, sizeof(char), 1, finput ) !=0) ) {
            printf("%X ", byte);

        }
    }
}

/*    int literal_length = 0;
    int copy_length = 0;
    const char *input_file_name = "C:\\Users\\belli\\Documents\\Archivio SUPSI\\SnappyProject\\asd20192020tpg3\\Snappy\\testWikipedia.txt";
    FILE *finput;
    char *input;
    char *output;
    const char * beginning;
    const char * out_beginning;
    char *input_limit;


    Tree *hash_table[htable_size];
    for (int i = 0; i < htable_size; i++) {
        hash_table[i] = create_tree();
    }

    if((finput = fopen(input_file_name, "r") )!= NULL){
        unsigned int file_size = getFileSize(input_file_name);
        input = (char *)calloc(file_size, sizeof(char));
        output = (char *)malloc(sizeof(char)*file_size);
        printf("LETTI:%d\n", file_size = fread(input, sizeof(char), file_size, finput) );
        printf("Dimesnione file: %d bytes, %d u32\n", file_size, file_size/4);
        beginning = input;
        out_beginning = output;
        input_limit = input + file_size;

        output = write_dim_varint(file_size, output);

    } else {
        puts("Errore apertura file");
        return 1;
    }
    fclose(finput);

    u32 current_u32 ;
    int index = 0;
    Node *copy;
    char *candidate;
    while( input+4 < input_limit ) {
        current_u32 = get_next_u32(input, input_limit);
        index = hash_bytes(current_u32);
        if (is_empty(hash_table[index]) | ((copy = find(current_u32, hash_table[index]) ) == NULL)) {
            //printf("%X literal\n", current_u32);
            //output[literal_length++] = current_u32; //Aggiungo il literal in output
            insert(current_u32, input - beginning , hash_table[index]);
            literal_length+=4;
            copy_length = 0;
        } else {

            output = write_literal(input - literal_length, output, literal_length);
            literal_length = 0;
            candidate = beginning + copy->offset;
            copy_length = find_copy_length(input+4, candidate + 4, input_limit);
            printf("%X copy of offset = %d and length = %d\n", current_u32, input - candidate, copy_length + 4);
            output = write_copy( output, copy_length + 4, input - candidate);
        }
        input+= 4 + copy_length;
    }
    output = write_literal(input - literal_length, output, literal_length);


    write_file_compressed(out_beginning, output);
    print_result_compression(finput, output, beginning, out_beginning, input_limit);*/
/*    puts("\n");
for (int i = 0; i < htable_size; ++i) {
    print_tree_inorder(hash_table[i]);
    printf("\n");
}*/


typedef struct buffer {
    char *current;
    char *beginning;
    unsigned int bytes_left;
} Buffer;

void init_Buffer(Buffer *bf, unsigned int buffer_size){
    bf->current = (char *)calloc(buffer_size, sizeof(char)); //TODO min?, init_buffer()?
    bf->beginning = bf->current;
    bf->bytes_left = buffer_size;
}
void move_current(Buffer *bf, unsigned int offset){
    bf->current += offset;
    bf->bytes_left -= offset;
}

typedef struct compressor{
    Tree **hash_table;
    u32 current_u32;
    int current_index;
    Node *copy; //TODO Che schifo?
} Compressor;

void init_compressor(Compressor *cmp){
    cmp->hash_table = (Tree **)malloc(sizeof(Tree*)*htable_size);
    for (int i = 0; i < htable_size; i++) {
        cmp->hash_table[i] = create_tree();
    }
}

static FILE *finput;
static FILE *fcompressed;
static unsigned long long file_size;
static Buffer input;
static Buffer output;
static Compressor cmp;
static unsigned int literal_length;




void get_file_size() {
    fseek(finput, 0, SEEK_END);
    file_size = ftell(finput);
    fseek(finput, 0, SEEK_SET);

/*    struct stat st;//TODO:?

    if (stat(filename, &st) == 0)
        return st.st_size;
    else return -1;*/
}
void open_file_input() {
    finput = fopen("..\\testWikipedia.txt", "rb");
    assert(finput != NULL);
    get_file_size();
}
void write_dim_varint() {
    unsigned int size_varint = parse_to_varint(file_size, output.current);
    output.current += size_varint;
}
void init_buffers() {
    init_Buffer(&input, MAX_BLOCK_SIZE);
    init_Buffer(&output, MAX_BLOCK_SIZE);
}
void load_next_block() {
    input.bytes_left = fread(input.current, sizeof(char), MAX_BLOCK_SIZE, finput);
}
int input_is_full() {
    return input.bytes_left != 0;
}
int is_block_end() {
    return input.bytes_left <= 15;//TODO, margine migliore?
}

u32 get_next_u32(const unsigned char *input) {
    return (input[0] << 24u) | (input[1] << 16u) | (input[2] << 8u) | input[3];
}

void generate_hash_index() {
    cmp.current_u32 = get_next_u32(input.current);
    cmp.current_index = hash_bytes(cmp.current_u32);
}

int found_match() {

    if ( !is_empty(cmp.hash_table[cmp.current_index]) ) {
        return (cmp.copy = find(cmp.current_u32, cmp.hash_table[cmp.current_index])) != NULL;//Salvo anche il nodo copia
    }
    return 0;
}



void start_new_literal() {
    literal_length = 0;
}
void append_literal() {
    literal_length += 4;
    move_current(&input, 4);
}

void exhaust_input() {

    literal_length += input.bytes_left;
    move_current(&input, input.bytes_left);

}


void update_hash_table() { //TODO prev hash & cur hash
    if(cmp.copy == NULL){
        u32 previous_u32 = get_next_u32(input.current-1); //Aggiungo anche u32 precedente per migliorare compressione
        //insert(previous_u32, input.current - input.beginning - 1, cmp.hash_table[hash_bytes(previous_u32)]);
        insert(cmp.current_u32, input.current - input.beginning , cmp.hash_table[cmp.current_index]);
    } else {
        cmp.copy->offset = input.current - input.beginning;
    }
}

void emit_literal() {
    output.current = write_literal(input.current - literal_length, output.current, literal_length);
}

void emit_copy() {
    char *candidate = input.beginning + cmp.copy->offset;
    int copy_length = 4 + find_copy_length(input.current + 4, candidate + 4, input.current + input.bytes_left);
    output.current = write_copy( output.current, copy_length, input.current - candidate);
    update_hash_table();
    printf("%X copy of offset = %d and length = %d\n",cmp.current_u32, input.current - candidate, copy_length);

    move_current(&input, copy_length);

}




void compress_next_block() {

    start_new_literal(); //All'inizio di ogni blocco c'? sempre un literal
    while(!is_block_end()){

        generate_hash_index();

        if(found_match()){//TODO: Heuristic match skipping
            emit_literal();
            start_new_literal();
            emit_copy();
        } else {
            update_hash_table();
            append_literal();
        }

    }
    exhaust_input();
    input.current - input.beginning;
    emit_literal();


}


void write_block_compressed() {
    fwrite(output.beginning, sizeof(char), output.current - output.beginning, fcompressed);
}

int main() {

    fcompressed = fopen("..\\test_compressed", "wb");

    open_file_input();
    init_compressor(&cmp);
    init_buffers();//TODO passare environment in parametro
    write_dim_varint();

    do {
        load_next_block();
        compress_next_block();
        write_block_compressed();
    } while(input_is_full());

    fclose(finput);
    fclose(fcompressed);
    print_result_compression(finput, output.current, input.beginning, output.beginning, input.current);




}


