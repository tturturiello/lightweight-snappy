#include <stdio.h>
#include "BST.h"

int main(){
    Tree *tree = create_tree();
    insert(32, 0, tree);
    insert(989, 0, tree);
    insert(12, 0, tree);
    insert(8, 0, tree);
    insert(99, 0, tree);
    insert(-1, 0, tree);

    print_tree_inorder(tree);

    Node *found_node = find(97889, tree);
    if(found_node != NULL)
        printf("\n%u", getBytes(found_node));

    free_tree(tree);
    //tree = create_tree();
    puts("");
    insert(99, 0, tree);
    insert(-1, 0, tree);

    print_tree_inorder(tree);


}
