#ifndef SNAPPY_BST_H
#define SNAPPY_BST_H

typedef unsigned u32;//TODO da spostare in un header apposito

typedef struct node {
    u32 bytes;
    unsigned long long offset;
    struct node *left,*right;
}Node;

struct tree {
    Node *root;
};

typedef struct tree Tree;
typedef struct node Node;

struct tree *create_tree();
void free_tree(Tree *tree);
int is_empty(Tree *tree);
void insert(u32 bytes, unsigned long long offset, Tree *tree);
void print_tree_inorder(Tree *tree);
Node * find(u32 bytes, Tree *tree);
u32 getBytes(Node * node);



#endif //SNAPPY_BST_H
