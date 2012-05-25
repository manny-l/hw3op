
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
	struct node_t *previous;
	lock nodeLock;
};

typedef struct node_t node;

struct  DoublyLinkedList_t
{
	node* HEAD;
	node* TAIL;
	lock listLock;
} ;

extern DoublyLinkedList list;

void Initialize ()
{
	(list) = (DoublyLinkedList)malloc(sizeof(struct DoublyLinkedList_t));
	if ((list) == NULL)
	{
		//printf("Error in memory allocation\n");
		exit(0);
	}
	//(*list)->root = NULL;
	//(*list)->isEmpty = 1;

	/*
	if (lock_init(&((*list)->treeLock))==-1)
	{
		free((*list));
		return -1;
	}
	*/

	list->HEAD=NULL;
	list->TAIL=NULL;

	return;
}


void Destroy()
{
	node* tmp;
	node* next;
	//The list is empty
	if (list->HEAD == NULL)
	{
		return;
	}
	//The list includes one node
	if (list->HEAD == list->TAIL)
	{
		free(list->HEAD);
		list->HEAD=NULL;
		list->TAIL=NULL;
		return;
	}
	//The list contains more then one node
	tmp = list->HEAD;
	while (tmp!=list->TAIL)
	{
		next=tmp->next;
		free(tmp);
		tmp = next;
	}
	list->HEAD=NULL;
	list->TAIL=NULL;
	return;
}

bool InsertHead (int key, char data)
{
	node* newNode;
	node* tmp;

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		return false;
	}
	newNode->key=key;
	newNode->unique=data;

	//The list is empty. after that the HEAD and TAIL
	//will point to the same node
	if (list->HEAD==NULL)
	{
		newNode->next=NULL;
		newNode->previous=NULL;
		list->HEAD=newNode;
		list->TAIL=newNode;
		//printf("%d\n",list->HEAD->key);
		return true;
	}

	//The list has one node
	if (list->HEAD==list->TAIL)
	{
		if (list->HEAD->key > key)
		{
			newNode->next=list->HEAD;
			newNode->previous=list->TAIL;
			list->HEAD=newNode;
			list->TAIL->previous=list->HEAD;
			list->TAIL->next=list->HEAD;
			return true;
		}
		if (list->HEAD->key < key)
		{
			newNode->next=list->TAIL;
			newNode->previous=list->HEAD;
			list->TAIL=newNode;
			list->HEAD->previous=list->TAIL;
			list->HEAD->next=list->TAIL;
			return true;
		}
		//list->HEAD->Key == key
		return false;
	}
	//The list is not empty
	tmp = list->HEAD;
	//replacing the Head
	if (list->HEAD->key > key)
	{
		newNode->next = list->HEAD;
		newNode->previous = list->HEAD->previous;
		newNode->previous->next=newNode;
		list->HEAD->previous = newNode;
		list->HEAD = newNode;
		return true;
	}
	//replacing the tail
	if (list->TAIL->key<key)
	{
		newNode->previous = list->TAIL;
		newNode->next = list->TAIL->next;
		list->TAIL->next = newNode;
		newNode->next->previous = newNode;
		list->TAIL = newNode;
		return true;
	}
	if ((list->HEAD->key==key) || (list->TAIL->key))
	{
		return false;
	}
	while (tmp!=list->TAIL)
	{
		if  (tmp->key<key)
		{
			newNode->previous = tmp;
			newNode->next = tmp->next;
			tmp->next = newNode;
			newNode->next->previous = newNode;
			//printf("check6");
			return true;
		}
		tmp=tmp->next;
	}
	//printf("reached end of inserthead\n");
	return false;
}

bool InsertTail(int key, char data)
{
	node* newNode;

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		return false;
	}
	newNode->key=key;
	newNode->unique=data;

	//The list is empty
	if (list->TAIL == NULL)
	{
		newNode->next = NULL;
		newNode->previous = NULL;
		list->HEAD = newNode;
		list->TAIL = newNode;
		return true;
	}

	//The list has one node
	if (list->HEAD==list->TAIL)
	{
		if (list->TAIL->key > key)
		{
			newNode->next=list->TAIL;
			newNode->previous=list->TAIL;
			list->HEAD=newNode;
			list->TAIL->previous=newNode;
			list->TAIL->next=newNode;
			return true;
		}
		if (list->TAIL->key < key)
		{
			newNode->next=list->HEAD;
			newNode->previous=list->HEAD;
			list->TAIL=newNode;
			list->HEAD->previous=newNode;
			list->HEAD->next=newNode;
			return true;
		}
		//list->HEAD->Key == key
		return false;
	}

	node *tmp = list->TAIL;
//
	//int flag = 0;


	//replacing the tail
	if (list->TAIL->key<key)
	{
		newNode->previous = list->TAIL;
		newNode->next = list->TAIL->next;
		list->TAIL->next = newNode;
		newNode->next->previous = newNode;
		list->TAIL = newNode;
		return true;
	}

	while (tmp!=list->HEAD)
	{
		if (tmp->key<key)
		{
			newNode->previous = tmp;
			newNode->next = tmp->next;
			tmp->next = newNode;
			newNode->next->previous = newNode;
		}
		tmp=tmp->next;
	}
	return false;
}

bool Delete(int key)
{
	node* tmp;
//	node* previous;
//	node* next;
	//the list is empty
	if (list->HEAD == NULL)
	{
		return false;
	}
	//The list has one node
	if (list->HEAD==list->TAIL)
	{
		if (list->HEAD->key==key)
		{
			tmp = list->HEAD;
			list->HEAD=NULL;
			list->TAIL=NULL;
			free(tmp);
			return true;
		} else
		{
			return false;
		}
	}
	tmp = list->HEAD;
	int flag = 0;
	while ((tmp!=list->HEAD) || (flag==0))
	{
		if (tmp->key == key)
		{
		/*	previous=tmp->previous;
			next=tmp->next;
			previous->next = tmp->next;
			next->previous = tmp->previous;*/

			tmp->previous->next = tmp->next;
			tmp->next->previous = tmp->previous;
			if (tmp==list->HEAD)
			{
				list->HEAD = tmp->next;
			}
			if (tmp == list->TAIL)
			{
				list->TAIL= tmp->previous;
			}
			free(tmp);
			return true;
		}
		tmp = tmp->next;
		flag = 1;
	}
	return false;
}

bool Search(int key, char* data)
{
	node* tmp;
	int flag = 0;
	//The list is empty
	if (list->HEAD == NULL)
	{
		return false;
	}
	//The list is not empty
	tmp = list->HEAD;
	while ((tmp!=list->HEAD) || (flag == 0))
	{
		if (tmp->key == key)
		{
			*data = tmp->unique;
			return true;
		}
		tmp = tmp->next;
		flag = 1;
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
	printf("list head key is %d and %c\n",list->HEAD->key,list->HEAD->unique);
	printf("retval is %s\n", (retval)? "true" : "false");
	printf("starting to destroy\n");
	Destroy();
	printf("finished\n");
	return 0;
	printf("start checking\n");
	Initialize();
	printf("finish Initializing\n");
	InsertTail(1,'a');
	printf("The first num is: %d\n",list->HEAD->key);
	if (list->HEAD!=list->TAIL)
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
	node* tmp=list->HEAD->next;
	printf("The second num is: %d\n",tmp->key);
	printf("The third num is: %d\n",list->HEAD->next->next->key);
	printf("The 4 num is: %d\n",list->HEAD->next->next->next->key);
	return 0;
}*/
