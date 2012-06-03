#include "myQueue.h"


void initQueue(queue q){
	q->first=NULL;
	q->last=NULL;
}

void insert(queue q, Action type,int pid)
{

	data new_data=(data)malloc(sizeof(struct queue_data));
	if (NULL==new_data)
	{
		exit(0);
	}
	new_data->type=type;
	new_data->next=NULL;
	new_data->pid = pid;

	//insert it to the queue
	if (NULL==q->last)
	{
		//if the queue is empty
		q->first=new_data;
		q->last=new_data;
	}
	else
	{
		q->last->next=new_data;
		q->last=new_data;
	}
}


bool remove_first(queue q)
{

	if (q->first==NULL)
	{
		return false;
	}

	data previous;
	previous=q->first;

	q->first=(q->first)->next;

	if (q->first==NULL)
	{
		q->last=NULL;
	}

	free(previous);

	return true;
}


void destroy_queue(queue q)
{
	data previous;
	while (q->first!=NULL)
	{
		previous=q->first;
		q->first=(q->first)->next;
		free(previous);
	}
	q->last=NULL;
}


bool is_empty(queue q, Action* type)
{
	if (q->first==NULL)
	{
		return true;
	}
	*type=(q->first)->type;
	return false;
}


