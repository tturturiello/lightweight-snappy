#ifndef SNAPPY_BST_H
#define SNAPPY_BST_H

typedef unsigned u32;//TODO da spostare in un header apposito

typedef struct node {
    u32 bytes;
    struct node *left,*right;
}Node;

struct tree {
    Node *root;
};

typedef struct tree Tree;
typedef struct node Node;

struct tree *create_tree();
void insert(u32 bytes, Tree *tree);
void print_tree_inorder(Tree *tree);
Node * find(u32 bytes, Tree *tree);
u32 getBytes(Node * node);


#endif //SNAPPY_BST_H
