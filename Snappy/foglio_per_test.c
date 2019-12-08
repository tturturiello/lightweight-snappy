#include <stdio.h>
#include "IO_utils.h"
#include "BST.h"
#include <stdlib.h>
#include <time.h>


int main(){

    srand(time(NULL));
    Tree *hash_table[10];
    for (int i = 0; i < 10; ++i) {
        hash_table[i] = create_tree();
        printf("%d\n", is_empty(hash_table[i]));
        insert(rand(), 0, hash_table[i]);
        insert(rand(), 0, hash_table[i]);
        insert(rand(), 0, hash_table[i]);
    }

    for (int i = 0; i < 10; ++i) {
        print_tree_inorder(hash_table[i]);
        printf("\n");
    }




}
