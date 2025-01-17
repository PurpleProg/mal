#include <stdio.h>
#include "reader.h"
#include "linked_list.h"


int main() {
    node_t * root = (node_t *)malloc(sizeof(node_t));

    char * hi = "aBcDeFg";
    root->data = hi;
    root->next = NULL;

    append(root, hi, sizeof(hi));

    char * str = (char *)root->data;
    printf("test : %s\n", str);
    str = (char *)root->next->data;
    printf("test : %s\n", str);


    node_t * tokens = tokenize(hi);



}
