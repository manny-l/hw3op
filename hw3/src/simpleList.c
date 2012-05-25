#include <stdio.h>
#include <stdlib.h>
#include "simpleList.h"

#define N 100
#define for_each_node(node, head) \
    for(node = head->next; node != NULL; node = node->next)

typedef enum command_en {
	INSHD,	//INSERT_HEAD
	INSTAIL,//INSERT_TAIL
	DEL,	//DELETE from list
	SEARCH	//SEARCH list
} commandType;

typedef struct fullCommand* commandp;
typedef struct list_head* headp;
typedef struct list_node* nodep;

typedef struct fullCommand{
	commandType comm;
	int key;
	int value;

} fullCommand_t;

struct list_node{
	commandp command;
	headp head;
	nodep next;
};

struct list_head{
	int numOfElements;
	nodep next;
};




nodep initNode(headp list_head,
			   commandType commnd,
			   int key,
			   int value){

	commandp newCommand=(commandp)malloc(sizeof(fullCommand_t));
	newCommand->comm=commnd;
	newCommand->key=key;
	newCommand->value=value;

	nodep newNode=(nodep)malloc(sizeof(struct list_node));
	newNode->command=newCommand;
	newNode->head=list_head;
	newNode->next=list_head->next;
	list_head->next=newNode;
	list_head->numOfElements++;
	return newNode;
}


headp listInit(){
	headp newList=(headp)malloc(sizeof(struct list_head));
	if (!newList) {
		printf("list init failed\n");
		return NULL;
	}
	newList->numOfElements = 0;
	newList->next = NULL;
	return newList;
}

int listIsEmpty(headp head){
	return (head->numOfElements ? 1 : 0);
}

void listInsert(headp head, commandType comm,
							int key,
							int value){
	nodep newNode=initNode(head,comm,key,value);
	newNode->next=head->next;
	head->next=newNode;
	head->numOfElements++;
	return;
}
/*
void listDelete(headp list_head, nodep node){
	if (node->head!=list_head){
		printf("head of node not compatible to given head");
		return;
	}
	nodep curr_node;
	commandp nodeCommand=node->command;
	commandp currCom;
	for_each_node(curr_node, list_head){
		currCom=curr_node->command;
		if (currCom->comm==nodeCommand->comm &&
			currCom->key==nodeCommand->key &&
			currCom->value==nodeCommand->value){
			//found command to delete
			free(node->command);

		}
	}
}
*/

