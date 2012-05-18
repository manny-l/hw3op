
#include <stdlib.h>
#unclude <stdio.h>

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

void Initialize (DoublyLinkedList *newList)
{
	newList->HEAD = (DoublyLinkedList) malloc(2*sizeof(struct node));
	newList->TAIL = (DoublyLinkedList) malloc(2*sizeof(struct node));
	newList->HEAD->next=NULL;
	newList->TAIL->next=NULL;
	newList->HEAD->previous=NULL;
	newList->TAIL->previous=NULL;
}

//How should i destroy it without parameters
void Destroy()
{
}

Bool InsertHead(int key, char data)
{	
	node newNode;
	if (list->HEAD!=NULL)
	{
		if (list->HEAD->key>key)
		{
			newNode = (node*) malloc(sizeof (struct node_t));
			if (newNode!=NULL)
			{
				newNode->key=key;
				newNode->unique=data;
				newNode->next=list->HEAD;
				newNode->previous=list->HEAD->previous;
				list->HEAD->previous = newNode;
				list->HEAD = newNode;
				return true;
			}
		} else
		{
			tmp = list->HEAD;
			while (tmp!=list->TAIL)
			{
				if (tmp->key == key)
				{
					return false;
				}
				if ((key>tmp->key) && (key<tmp->next->key))
				{
					newNode = (node*) malloc(sizeof (struct node_t));
					if (newNode!=NULL)
					{
						newNode->key=key;
						newNode->unique=data;
						newNode->next=tmp->next;
						newNode->previous=tmp;
						tmp->next=newNode;
						tmp->next->previous=newNode;
						return true;
					}
				}
			}
		}
	}
		
}

Bool InsertTail(int key, char data)
{	
	node newNode;
	if (list->TAIL!=NULL)
	{
		if (list->TAIL->key>key)
		{
			newNode = (node*) malloc(sizeof (struct node_t));
			if (newNode!=NULL)
			{
				newNode->key=key;
				newNode->unique=data;
				newNode->next=list->HEAD;
				newNode->previous=list->HEAD->previous;
				list->HEAD->previous = newNode;
				list->HEAD = newNode;
				return true;
			}
		} else
		{
			tmp = list->HEAD;
			while (tmp!=list->TAIL)
			{
				if (tmp->key == key)
				{
					return false;
				}
				if ((key>tmp->key) && (key<tmp->next->key))
				{
					newNode = (node*) malloc(sizeof (struct node_t));
					if (newNode!=NULL)
					{
						newNode->key=key;
						newNode->unique=data;
						newNode->next=tmp->next;
						newNode->previous=tmp;
						tmp->next=newNode;
						tmp->next->previous=newNode;
						return true;
					}
				}
			}
		}
	}
		
}

Bool Delete(int key)
{	
	node* tmp, previous, next;
	if (list->HEAD == NULL)
	{
		return false;
	}
	tmp = list->HEAD;
	while (tmp!=list->TAIL)
	{
		if (tmp->key == key)
		{
			previous=tmp->previous;
			next=tmp->next;
			previous->next = tmp->next;
			next->previous = tmp->previous;
			free(tmp);
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}

Bool search(int key, char* data)
{
	node* tmp;
		if (list->HEAD == NULL)
	{
		return false;
	}
	tmp = list->HEAD;
	while (tmp!=list->TAIL)
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

