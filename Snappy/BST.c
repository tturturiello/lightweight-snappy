#include <stdlib.h>
#include <stdio.h>
#include "BST.h"



u32 getBytes(Node * node){
    return node->bytes;
}

struct tree *create_tree(){
    struct tree *tree = (struct tree *)malloc(sizeof(struct tree));
    tree->root = NULL;
    return tree;
}

int is_empty(Tree *tree){
    return tree->root == NULL;
}

Node *insert_node(u32 bytes, unsigned long long offset ,Node *node){

    if (node == NULL) {
        node = (Node *)malloc(sizeof(Node));
        node->bytes = bytes;
        node->offset = offset;
        node->left = node->right = NULL;
    } else if (bytes < node->bytes) {
        node->left= insert_node(bytes, offset, node->left);
    } else if (bytes > node->bytes) {
        node->right= insert_node(bytes, offset, node->right);
    }
    return node;
}

void insert(u32 bytes, unsigned long long offset, Tree *tree){
    tree->root = insert_node(bytes, offset, tree->root);
}

void print_node(Node *node) {
    if(node!=NULL){
        print_node(node->left);
        printf("[%X, %llu] ", node->bytes, node->offset);
        print_node(node->right);
    }
}

void print_tree_inorder(Tree *tree){
    print_node(tree->root);
}


Node *find_node(Node *node, u32 bytes)
{
    if (!node) {
        return NULL;
    }
    if (bytes < node->bytes) {
        return find_node(node->left, bytes);
    } else if (bytes > node->bytes) {
        return find_node(node->right, bytes);
    } else {
        return node;
    }
}

Node * find(u32 bytes, Tree *tree){
    return find_node(tree->root, bytes);
}