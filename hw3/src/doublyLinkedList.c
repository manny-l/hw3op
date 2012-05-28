
/*
 * doublyLinkedList.c
 *
 *  Created on: May 18, 2012
 *      Author: root
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "doublyLinkedList.h"
#include "lock.h"


struct node_t
{
	int key;
	char unique;
	struct node_t *next;
	struct node_t *prev;
	lock nodeLock;
};

typedef struct node_t node;

struct  DoublyLinkedList_t
{
	node* head;
	node* tail;
	lock listLock;
} ;

extern DoublyLinkedList list;

void Initialize ()
{
	node* newNode;

	(list) = (DoublyLinkedList)malloc(sizeof(struct DoublyLinkedList_t));
	if ((list) == NULL)
	{
		//printf("Error in memory allocation\n");
		exit(0);
	}

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		exit(0);
	}

	newNode->key=0;
	newNode->unique='0';
	newNode->next = newNode;
	newNode->prev = newNode;
	list->head=newNode;
	list->tail=newNode;

	//(*list)->isEmpty = 1;

	if (lock_init(&(list->listLock))==-1)
	{
		free(newNode);
		free(list);
		printf("not ok\n");
		exit(0);
	}

	return;
}


void Destroy()
{
	node* tmp;
	node* next;
	//The list is empty
	if (list->head == NULL)
	{
		return;
	}
	//The list includes one node
	if (list->head == list->tail)
	{
		free(list->head);
		list->head=NULL;
		list->tail=NULL;
		return;
	}
	//The list contains more then one node
	tmp = list->head;
	while (tmp!=list->tail)
	{
		next=tmp->next;
		free(tmp);
		tmp = next;
	}
	list->head=NULL;
	list->tail=NULL;
	return;
}

bool InsertHead (int key, char data)
{
	node* newNode;
	node* tmp;
	int tempKey;
	node* currHead;

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		exit(0);
	}
	newNode->key=key;
	newNode->unique=data;

	//The list is empty: node 0 is found.
	if (list->head==list->tail) //key 0
	{
		//get_write_lock(&(list->listLock));

		//check if list is still empty
		if (list->head==list->tail)
		{
			list->head->next = newNode;
			list->head->prev = newNode;

			newNode->next=list->head;
			newNode->prev=list->head;

			list->tail = newNode;

			//printf("%d\n",list->head->key);
			//release_exclusive_lock(&(list->listLock));
			return true;
		}

		//release_exclusive_lock(&(list->listLock));
	}

	//one node or more
	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key==key)
		{
			free(newNode);
			return false;
		}
		else if  (tmp->key>key)
		{
			newNode->prev = tmp->prev;
			tmp->prev->next = newNode;

			newNode->next = tmp;
			tmp->prev = newNode;

			return true;
		}
		tmp=tmp->next;
	}

	//Replace tail
	list->tail->next = newNode;
	list->head->prev = newNode;

	newNode->next = list->head;
	newNode->prev = list->tail;

	list->tail = newNode;
	return true;
}

bool InsertTail(int key, char data)
{
	node* newNode;
	node* tmp;
	int tempKey;
	node* currTail;

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		exit(0);
	}
	newNode->key=key;
	newNode->unique=data;

	//The list is empty: node 0 is found.
	if (list->head==list->tail) //key 0
	{
		//get_write_lock(&(list->listLock));

		//check if list is still empty
		if (list->head==list->tail)
		{
			list->head->next = newNode;
			list->head->prev = newNode;

			newNode->next=list->head;
			newNode->prev=list->head;

			list->tail = newNode;

			//printf("%d\n",list->head->key);
			//release_exclusive_lock(&(list->listLock));
			return true;
		}

		//release_exclusive_lock(&(list->listLock));
	}

	//one node or more
	if (list->tail->key==key)
	{
		free(newNode);
		return false;
	}

	tmp = list->tail->prev;

	while (tmp!=list->tail)
	{
		if (tmp->key==key)
		{
			free(newNode);
			return false;
		}
		else if  (tmp->key<key)
		{
			newNode->prev = tmp;
			newNode->next = tmp->next;

			tmp->next->prev = newNode;
			tmp->next = newNode;

			return true;
		}
		tmp=tmp->prev;
	}

	printf("SHOULDNT SEE ME\n");
	return false;
}

bool Delete(int key)
{
	node* tmp;
//	node* prev;
//	node* next;
	//the list is empty
	if (list->head == list->tail)
	{
		return false;
	}

	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key == key)
		{
			tmp->prev->next = tmp->next;
			tmp->next->prev = tmp->prev;

			if (tmp == list->tail)
			{
				list->tail= tmp->prev;
				list->tail->next = list->head;
				list->head->prev = list->tail;
			}
			free(tmp);
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}

bool Search(int key, char* data)
{
	node* tmp;
	//The list is empty
	if (list->head == list->tail)
	{
		//printf("my list is empty\n");
		return false;
	}
	//The list is not empty
	tmp = list->head->next;
	while (tmp!=list->head)
	{
		if (tmp->key == key)
		{
			*data = tmp->unique;
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}


/*int main ()
{
	printf("hello 0\n");
	bool retval;
	Initialize();
	printf("hello 1\n");
	retval=InsertHead(5,'a');
	printf("hello 2\n");
	printf("list head key is %d and %c\n",list->head->key,list->head->unique);
	printf("retval is %s\n", (retval)? "true" : "false");
	printf("starting to destroy\n");
	Destroy();
	printf("finished\n");
	return 0;
	printf("start checking\n");
	Initialize();
	printf("finish Initializing\n");
	InsertTail(1,'a');
	printf("The first num is: %d\n",list->head->key);
	if (list->head!=list->tail)
	{
		printf("error: the head and tail are different");
	}
	InsertTail(2,'b');
	char* data;
	if (Search(4,data)!=false)
	{
		printf("error in search");
	}
	InsertHead(4,'d');
	if (Search(4,data)==false)
	{
		printf("error in search");
	}
	Delete(4);
	InsertTail(5,'e');
	node* tmp=list->head->next;
	printf("The second num is: %d\n",tmp->key);
	printf("The third num is: %d\n",list->head->next->next->key);
	printf("The 4 num is: %d\n",list->head->next->next->next->key);
	return 0;
}*/
