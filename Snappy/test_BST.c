#include <stdio.h>
#include "BST.h"

int main(){
    Tree *tree = create_tree();
    insert(32, tree);
    insert(989, tree);
    insert(12, tree);
    insert(8, tree);
    insert(99, tree);
    insert(-1, tree);

    print_tree_inorder(tree);

    Node *found_node = find(989, tree);
    if(found_node != NULL)
        printf("\n%u", getBytes(found_node));
}
