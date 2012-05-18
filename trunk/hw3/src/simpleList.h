/*
 * simpleList.h
 *
 *  Created on: May 18, 2012
 *      Author: root
 */

#ifndef SIMPLELIST_H_
#define SIMPLELIST_H_

#define N 100
#define for_each_node(node, head) \
    for(node = head->next; node != NULL; node = node->next)

typedef enum command_t {
	INSHD,	//insert_head
	INSTAIL,//insert_tail
	DEL,	//delete from list
	SEARCH	//search list
} command;

typedef struct list_head* headp;
typedef struct list_node* nodep;

struct command{

};

struct list_node{
	char command[N];
	headp head;
	nodep next;
};

struct list_head{
	int numOfElements;
	nodep next;
};

headp listInit(){
	headp newlist=malloc(sizeof(headp));
	if (!newlist) {
		printf("list init failed\n");
		return -1;
	}
	newlist->numOfElements = 0;
	newlist->next=NULL;
	return newlist;
}

int listIsEmpty(headp head){
	return (head->numOfElements ? 1 : 0);
}

void listInsert(headp head, char com[N]){
	nodep newNode;
	if (!listIsEmpty(head)){
		for_each_node(newNode,head){
			if (!strcmp(newNode->command,com)) return 0;
		}
	}
	if (newNode){
		if (!strcmp(newNode->command,com)) return 0;
		while (newNode->next){
			if (!strcmp(newNode->command,com)) return 0;
			newNode->next=newNode->next->next;
		}
	}
	newNode=malloc(sizeof(nodep));
	newNode->head=head;
	newNode->next=head->next;
	head->next=newNode;
	head->numOfElements++;
	strcpy(newNode->command,com);
	return 0;

}

#endif /* SIMPLELIST_H_ */
