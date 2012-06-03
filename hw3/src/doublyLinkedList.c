/*
 * doublyLinkedList.c
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

	(list) = (DoublyLinkedList)malloc
		(sizeof(struct DoublyLinkedList_t));
	if ((list) == NULL)
	{
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

	if (lock_init(&(list->head->nodeLock))==-1)
	{
		free(newNode);
		free(list);
		exit(0);
	}

	if (lock_init(&(list->listLock))==-1)
	{
		free(newNode);
		free(list);
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

	//The list contains more then one node
	tmp = list->head->next;
	while (tmp!=list->head)
	{
		next=tmp->next;
		lock_destroy(&(tmp->nodeLock));
		free(tmp);
		tmp = next;
	}

	lock_destroy(&(list->head->nodeLock));
	free(list->head);

	lock_destroy(&(list->listLock));

	free(list);

	return;
}

bool InsertHead (int key, char data)
{
	node* newNode;
	node* tmp;

	newNode = (node*) malloc(sizeof (struct node_t));

	if (newNode==NULL)
	{
		exit(0);
	}
	newNode->key=key;
	newNode->unique=data;

	if (lock_init(&(newNode->nodeLock))==-1)
	{
		free(newNode);
		printf("not ok\n");
		exit(0);
	}


	//The list is empty: node 0 is found.
	if (list->head==list->tail) //key 0
	{
		get_write_lock(&(list->listLock));
		get_write_lock(&(list->head->nodeLock));

		//check if list is still empty
		if (list->head==list->tail)
		{
			list->head->next = newNode;
			list->head->prev = newNode;

			newNode->next=list->head;
			newNode->prev=list->head;

			list->tail = newNode;

			release_exclusive_lock(&(list->head->nodeLock));
			release_exclusive_lock(&(list->listLock));

			return true;
		}
		release_exclusive_lock(&(list->head->nodeLock));
		release_exclusive_lock(&(list->listLock));
	}

	//one node or more

	get_may_write_lock(&(list->head->nodeLock));
	get_may_write_lock(&(list->head->next->nodeLock));

	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key==key)
		{
			free(newNode);
			release_shared_lock(&(tmp->nodeLock));
			release_shared_lock(&(tmp->prev->nodeLock));

			return false;

		}
		else if  (tmp->key>key)
		{
			upgrade_may_write_lock(&(tmp->nodeLock));
			upgrade_may_write_lock(&(tmp->prev->nodeLock));

			newNode->prev = tmp->prev;
			tmp->prev->next = newNode;

			newNode->next = tmp;
			tmp->prev = newNode;

			release_exclusive_lock(&(newNode->prev->nodeLock));
			release_exclusive_lock(&(newNode->next->nodeLock));

			return true;
		}

		release_shared_lock(&(tmp->prev->nodeLock));
		get_may_write_lock(&(tmp->next->nodeLock));
		tmp=tmp->next;
	}

	//Replace tail

	upgrade_may_write_lock(&(list->head->nodeLock));
	upgrade_may_write_lock(&(list->tail->nodeLock));

	get_write_lock(&(list->listLock));

	list->tail->next = newNode;
	list->head->prev = newNode;

	newNode->next = list->head;
	newNode->prev = list->tail;

	list->tail = newNode;
	release_exclusive_lock(&(list->listLock));

	release_exclusive_lock(&(list->tail->prev->nodeLock));
	release_exclusive_lock(&(list->head->nodeLock));

	return true;
}

bool InsertTail(int key, char data)
{
	node* newNode;
	node* tmp;

	newNode = (node*) malloc(sizeof (struct node_t));
	if (newNode==NULL)
	{
		exit(0);
	}
	newNode->key=key;
	newNode->unique=data;

	if (lock_init(&(newNode->nodeLock))==-1)
	{
		free(newNode);
		printf("not ok\n");
		exit(0);
	}

	//The list is empty: node 0 is found.
	if (list->head==list->tail) //key 0
	{
		get_write_lock(&(list->listLock));
		get_write_lock(&(list->head->nodeLock));

		//check if list is still empty
		if (list->head==list->tail)
		{
			list->head->next = newNode;
			list->head->prev = newNode;

			newNode->next=list->head;
			newNode->prev=list->head;

			list->tail = newNode;

			release_exclusive_lock(&(list->listLock));
			release_exclusive_lock(&(list->head->nodeLock));
			return true;
		}

		release_exclusive_lock(&(list->listLock));
		release_exclusive_lock(&(list->head->nodeLock));
	}

	//one node or more
	get_may_write_lock(&(list->head->nodeLock));
	get_may_write_lock(&(list->tail->nodeLock));

	tmp = list->tail;

	bool contFlag = true;

	while ((tmp!=list->tail) || (contFlag=true))
	{
		contFlag=false;
		if (tmp->key==key)
		{
			free(newNode);
			release_shared_lock(&(tmp->nodeLock));
			release_shared_lock(&(tmp->next->nodeLock));
			return false;
		}
		else if  (tmp->key<key)
		{
			upgrade_may_write_lock(&(tmp->nodeLock));
			upgrade_may_write_lock(&(tmp->next->nodeLock));

			newNode->prev = tmp;
			newNode->next = tmp->next;

			tmp->next->prev = newNode;
			tmp->next = newNode;

			if (tmp==list->tail)
			{
				get_write_lock(&(list->listLock));
				list->tail=newNode;
				list->head->prev = newNode;
				release_exclusive_lock(&(list->listLock));
			}

			release_exclusive_lock(&(newNode->prev->nodeLock));
			release_exclusive_lock(&(newNode->next->nodeLock));

			return true;
		}

		release_shared_lock(&(tmp->next->nodeLock));
		get_may_write_lock(&(tmp->prev->nodeLock));

		tmp=tmp->prev;

	}

	printf("SHOULDNT SEE ME\n");
	return false;
}

bool Delete(int key)
{
	node* tmp;
	//the list is empty
	if (list->head == list->tail)
	{
		return false;
	}

	get_may_write_lock(&(list->head->next->nodeLock));
	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key == key)
		{

			if (tmp->prev->key != tmp->next->key)
			{
				get_write_lock(&(tmp->prev->nodeLock));
			}

			upgrade_may_write_lock(&(tmp->nodeLock));

			get_write_lock(&(tmp->next->nodeLock));

			tmp->prev->next = tmp->next;
			tmp->next->prev = tmp->prev;

			if (tmp == list->tail)
			{
				get_write_lock(&(list->listLock));
				list->tail= tmp->prev;
				list->tail->next = list->head;
				list->head->prev = list->tail;
				release_exclusive_lock(&(list->listLock));
			}

			if (tmp->prev->key != tmp->next->key)
			{
				release_exclusive_lock(&(tmp->prev->nodeLock));
			}

			release_exclusive_lock(&(tmp->next->nodeLock));

			release_exclusive_lock(&(tmp->nodeLock));
			lock_destroy(&(tmp->nodeLock));
			free(tmp);

			return true;
		}
		release_shared_lock(&(tmp->nodeLock));
		tmp = tmp->next;
		get_read_lock(&(tmp->nodeLock));
	}

	release_shared_lock(&(tmp->nodeLock));
	return false;
}

bool Search(int key, char* data)
{
	node* tmp;
	//The list is empty
	if (list->head == list->tail)
	{
		return false;
	}
	//The list is not empty

	get_read_lock(&(list->head->next->nodeLock));
	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key == key)
		{
			*data = tmp->unique;
			release_shared_lock(&(tmp->nodeLock));
			return true;
		}

		release_shared_lock(&(tmp->nodeLock));
		get_read_lock(&(tmp->next->nodeLock));

		tmp = tmp->next;

	}

	release_shared_lock(&(tmp->nodeLock));
	return false;
}

