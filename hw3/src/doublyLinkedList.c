
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

	if (lock_init(&(list->head->nodeLock))==-1)
	{
		free(newNode);
		free(list);
		printf("not ok\n");
		exit(0);
	}

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

	//list->head=NULL;
	//list->tail=NULL;
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

	if (lock_init(&(newNode->nodeLock))==-1)
	{
		free(newNode);
		printf("not ok\n");
		exit(0);
	}


	//The list is empty: node 0 is found.
	if (list->head==list->tail) //key 0
	{
		//printf("trying to lock list\n");
		get_write_lock(&(list->listLock));

		//printf("trying to lock %d\n", list->head->key);
		get_write_lock(&(list->head->nodeLock));


		//check if list is still empty
		if (list->head==list->tail)
		{
			list->head->next = newNode;
			list->head->prev = newNode;

			newNode->next=list->head;
			newNode->prev=list->head;

			list->tail = newNode;

			//printf("%d\n",list->head->key);
			//printf("releasing lock %d\n",list->head->key);
			release_exclusive_lock(&(list->head->nodeLock));
			//printf("releasing list lock\n");
			release_exclusive_lock(&(list->listLock));

			return true;
		}
		//printf("releasing %d\n", list->head->key);
		release_exclusive_lock(&(list->head->nodeLock));
		//printf("releasing list lock\n");
		release_exclusive_lock(&(list->listLock));
	}

	//one node or more

	//printf("trying to lock %d\n", list->head->key);
	get_may_write_lock(&(list->head->nodeLock));

	//printf("trying to lock %d\n", list->head->next->key);
	get_may_write_lock(&(list->head->next->nodeLock));


	tmp = list->head->next;

	while (tmp!=list->head)
	{
		if (tmp->key==key)
		{
			free(newNode);
			//printf("releasing %d\n", tmp->key);
			release_shared_lock(&(tmp->nodeLock));
			//printf("releasing %d\n", tmp->prev->key);
			release_shared_lock(&(tmp->prev->nodeLock));

			return false;

		}
		else if  (tmp->key>key)
		{
			//printf("Enter between\n")
			//printf("upgrade %d\n", tmp->key);
			upgrade_may_write_lock(&(tmp->nodeLock));
			//printf("upgrade %d\n", tmp->prev->key);
			upgrade_may_write_lock(&(tmp->prev->nodeLock));

			newNode->prev = tmp->prev;
			tmp->prev->next = newNode;

			newNode->next = tmp;
			tmp->prev = newNode;

			//printf("releasing %d\n", newNode->prev->key);
			release_exclusive_lock(&(newNode->prev->nodeLock));
			//printf("releasing %d\n", newNode->next->key);
			release_exclusive_lock(&(newNode->next->nodeLock));

			return true;
		}

		//printf("releasing %d\n", tmp->prev->key);
		release_shared_lock(&(tmp->prev->nodeLock));

		//printf("trying to lock %d\n", tmp->next->key);
		get_may_write_lock(&(tmp->next->nodeLock));


		tmp=tmp->next;
	}

	//Replace tail
	//get_write_lock(&(list->head->nodeLock));
	//get_write_lock(&(list->tail->nodeLock));

	upgrade_may_write_lock(&(list->head->nodeLock));
	upgrade_may_write_lock(&(list->tail->nodeLock));


	//printf("Adding to tail\n");

	get_write_lock(&(list->listLock));

	list->tail->next = newNode;
	list->head->prev = newNode;

	newNode->next = list->head;
	newNode->prev = list->tail;

	//printf("trying to lock list\n");


	list->tail = newNode;
	//printf("releasing list\n");
	release_exclusive_lock(&(list->listLock));

	//printf("releasing %d\n",list->tail->prev->key);
	release_exclusive_lock(&(list->tail->prev->nodeLock));
	//printf("releasing %d\n",list->head->key);
	release_exclusive_lock(&(list->head->nodeLock));

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

			//printf("%d\n",list->head->key);
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
//	node* prev;
//	node* next;;
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

			upgrade_may_write_lock(&(tmp->nodeLock));

			if (tmp->prev->key != tmp->next->key)
			{
				get_write_lock(&(tmp->prev->nodeLock));
			}

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
		//printf("my list is empty\n");
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
