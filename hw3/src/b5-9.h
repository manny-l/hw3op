#ifndef _5_9_TREE_H
#define _5_9_TREE_H

#include "lock.h"

typedef struct tree_t* tree;

int initTree(tree* tr);
int addElm(tree tr,int key,char data);
int findElm(tree tr,int key,char* data);
void freeTree(tree tr);

#endif //end of _5-9_TREE_H
