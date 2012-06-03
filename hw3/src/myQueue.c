#include "myQueue.h"

/*----------------------------------------------------------------
initQueue function:
Input\Output: q - the current Queue
Return value: NONE
Description: initialization of a new Queue
-----------------------------------------------------------------*/
void initQueue(queue q){
	q->first=NULL;
	q->last=NULL;
}

/*----------------------------------------------------------------
insert function:
Input:		  type - the action of the new element in the queue
			  pid  - the pid of the new element in the queue
Input\Output: q    - the current Queue
Return value: NONE
Description:- allocate a new queue_data element
			- enter the given type & pid to the new element
			- add the new element to the given queue
-----------------------------------------------------------------*/
void insert(queue q, Action type,int pid)
{
	//create a new node for the data
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

/*----------------------------------------------------------------
remove_first function:
Input\Output: q    - the current Queue
Return value: true - upon success, false otherwise
Description:- allocate a new queue_data element
			- enter the given type & pid to the new element
			- add the new element to the given queue
-----------------------------------------------------------------*/
bool remove_first(queue q)
{
	//check if the queue is empty
	if (q->first==NULL)
	{
		return false;
	}

	//save the pointer to the first node
	data previous;
	previous=q->first;

	//move to the next node
	q->first=(q->first)->next;

	//if the queue is empty
	if (q->first==NULL)
	{
		q->last=NULL;
	}

	//delete the first node
	free(previous);

	return true;
}

/*----------------------------------------------------------------
destroy_queue function:
Input\Output: q    - the current Queue
Return value: NONE
Description: going through the queue & freeing all the queue_data
-----------------------------------------------------------------*/
void destroy_queue(queue q)
{
	data previous;
	while (q->first!=NULL)
	{
		previous=q->first;
		//move to the next node
		q->first=(q->first)->next;
		//delete the first node
		free(previous);
	}
	q->last=NULL;
}

/*----------------------------------------------------------------
is_empty function:
Input\Output: q  - the current Queue
Output:		  type - the Action of the first node in the queue
Return value: true - if the queue is empty, false otherwise
Description: -if the true is empty returns true
             - else, returns false, and the first Action in type
-----------------------------------------------------------------*/
bool is_empty(queue q, Action* type)
{
	if (q->first==NULL)
	{
		return true;
	}
	*type=(q->first)->type;
	return false;
}


