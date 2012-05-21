
/*
 * doublyLinkedList.c
 *
 *  Created on: May 18, 2012
 *      Author: root
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


struct node_t
{
	int key;
	char unique;
	struct node_t *next;
	struct node_t *previous;
};

typedef struct node_t node;

struct  DoublyLinkedList_t
{
	node* HEAD;
	node* TAIL;
} ;
typedef struct DoublyLinkedList_t DoublyLinkedList;

DoublyLinkedList list;


void Initialize ()
{
	list.HEAD=NULL;
	list.TAIL=NULL;
}


void Destroy()
{
	node* tmp;
	node* next;
	//The list is empty
	if (list.HEAD == NULL)
	{
		return;
	}
	//The list includes one node
	if (list.HEAD == list.TAIL)
	{
		free(tmp);
		list.HEAD=NULL;
		list.TAIL=NULL;
		return;
	}
	//The list contains more then one node
	tmp = list.HEAD;
	while (tmp!=list.TAIL)
	{
		next=tmp;
		free(tmp);
		tmp = next;
	}
	list.HEAD=NULL;
	list.TAIL=NULL;
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
	if (list.HEAD==NULL)
	{
		newNode->next=NULL;
		newNode->previous=NULL;
		list.HEAD=newNode;
		list.TAIL=newNode;
		printf("%d\n",list.HEAD->key);
		return true;
	}

	//The list has one node
	if (list.HEAD==list.TAIL)
	{
		if (list.HEAD->key > key)
		{
			newNode->next=list.HEAD;
			newNode->previous=list.TAIL;
			list.HEAD=newNode;
			list.TAIL->previous=list.HEAD;
			list.TAIL->next=list.HEAD;
			printf("check2\n");
			return true;
		}
		if (list.HEAD->key < key)
		{
			newNode->next=list.TAIL;
			newNode->previous=list.HEAD;
			list.TAIL=newNode;
			list.HEAD->previous=list.TAIL;
			list.HEAD->next=list.TAIL;
			return true;
		}
		//list->HEAD->Key == key
		return false;
	}

	//The list is not empty
	tmp = list.HEAD;
	//replacing the Head
	if (list.HEAD->key > key)
	{
		newNode->next = list.HEAD;
		newNode->previous = list.HEAD->previous;
		newNode->previous->next=newNode;
		list.HEAD->previous = newNode;
		list.HEAD = newNode;
		printf("check4\n");
		return true;
	}
	//replacing the tail
	if (list.TAIL->key<key)
	{
		newNode->previous = list.TAIL;
		newNode->next = list.TAIL->next;
		list.TAIL->next = newNode;
		newNode->next->previous = newNode;
		list.TAIL = newNode;
		printf("%d\n",list.HEAD->next->next->key);
		return true;
	}
	if ((list.HEAD->key==key) || (list.TAIL->key))
	{
		return false;
	}
	while (tmp!=list.TAIL)
	{
		if  (tmp->key<key)
		{
			newNode->previous = tmp;
			newNode->next = tmp->next;
			tmp->next = newNode;
			newNode->next->previous = newNode;
			printf("check6");
			return true;
		}
		tmp=tmp->next;
	}
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
	if (list.TAIL == NULL)
	{
		newNode->next = NULL;
		newNode->previous = NULL;
		list.HEAD = newNode;
		list.TAIL = newNode;
		return true;
	}

	//The list has one node
	if (list.HEAD==list.TAIL)
	{
		if (list.TAIL->key > key)
		{
			newNode->next=list.HEAD;
			newNode->previous=list.TAIL;
			list.HEAD=newNode;
			list.TAIL->previous=list.HEAD;
			list.TAIL->next=list.HEAD;
			return true;
		}
		if (list.HEAD->key < key)
		{
			newNode->next=list.TAIL;
			newNode->previous=list.TAIL;
			list.TAIL=newNode;
			list.HEAD->previous=list.HEAD;
			list.HEAD->next=list.HEAD;
			return true;
		}
		//list->HEAD->Key == key
		return false;
	}

	node *tmp = list.TAIL;
//
	//int flag = 0;


	//replacing the tail
	if (list.TAIL->key<key)
	{
		newNode->previous = list.TAIL;
		newNode->next = list.TAIL->next;
		list.TAIL->next = newNode;
		newNode->next->previous = newNode;
		list.TAIL = newNode;
		return true;
	}

	while (tmp!=list.HEAD)
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
	if (list.HEAD == NULL)
	{
		return false;
	}
	//The list has one node
	if (list.HEAD==list.TAIL)
	{
		if (list.HEAD->key==key)
		{
			tmp = list.HEAD;
			list.HEAD=NULL;
			list.TAIL=NULL;
			free(tmp);
			return true;
		} else
		{
			return false;
		}
	}
	tmp = list.HEAD;
	int flag = 0;
	while ((tmp!=list.HEAD) || (flag==0))
	{
		if (tmp->key == key)
		{
		/*	previous=tmp->previous;
			next=tmp->next;
			previous->next = tmp->next;
			next->previous = tmp->previous;*/

			tmp->previous->next = tmp->next;
			tmp->next->previous = tmp->previous;
			if (tmp==list.HEAD)
			{
				list.HEAD = tmp->next;
			}
			if (tmp == list.TAIL)
			{
				list.TAIL= tmp->previous;
			}
			free(tmp);
			return true;
		}
		tmp = tmp->next;
		flag = 1;
	}
	return false;
}

bool search(int key, char* data)
{
	node* tmp;
	int flag = 0;
	//The list is empty
	if (list.HEAD == NULL)
	{
		return false;
	}
	//The list is not empty
	tmp = list.HEAD;
	while ((tmp!=list.HEAD) || (flag == 0))
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

/*
int main ()
{
	printf("start checking\n");
	Initialize();
	printf("finish Initializing\n");
	InsertTail(1,'a');
	InsertTail(2,'b');
	InsertTail(4,'d');
	printf("The first num is: %d\n",list.HEAD->key);
	node* tmp=list.TAIL->next;
	printf("The second num is: %d\n",tmp->key);
	printf("The third num is: %d\n",list.HEAD->next->next->key);
	return 0;
}
*/
