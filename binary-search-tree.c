#include <stdio.h>
#include <stdlib.h>

struct node {
    int val;
    struct node *left, *right;
};

/* initialize root node if it is NULL, otherwise set left or right and return node */

struct node *add(struct node *root, int val){

    struct node *new = malloc(sizeof(struct node));
    struct node *initial_root = root;

    // initialize new
    new->val = val;
    new->left = NULL;
    new->right = NULL;

    if(root == NULL){
        return new;
    } 
    
    while(1){

        if (val > root->val){
            if(root->right == NULL) {
                root->right = new;
                return initial_root;
            }
            root = root->right;
        } else{
             if(root->left == NULL) {
                root->left = new;
                return initial_root;
            }
            root = root->left;
        }
    }
};


void print_sorted(struct node *root){
    if(root == NULL) return;

    if(root->left != NULL){
        print_sorted(root->left);
    } 
    printf("%d\n", root->val);
    
    if(root->right != NULL){
        print_sorted(root->right);
    } 
};

int main(void){

    struct node *root = NULL;

    root = add(root, 5);
    root = add(root, 20);
    root = add(root, 10);
    root = add(root, 40);
    root = add(root, 33);

    print_sorted(root);

    return 0;
}