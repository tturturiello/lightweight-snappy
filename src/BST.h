#ifndef SNAPPY_BST_H
#define SNAPPY_BST_H

typedef unsigned u32;

typedef struct node {
    u32 bytes;
    unsigned long long offset;
    struct node *left,*right;
}Node;

typedef struct tree {
    Node *root;
} Tree;


Tree *create_tree();
void free_tree(Tree *tree);
int is_empty(Tree *tree);
void insert(u32 bytes, unsigned long long offset, Tree *tree);
void print_tree_inorder(Tree *tree);
Node * find(u32 bytes, Tree *tree);
u32 getBytes(Node * node);



#endif //SNAPPY_BST_H
