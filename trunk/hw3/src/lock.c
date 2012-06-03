#include "lock.h"

void get_new_thread(lock *l,queue q);

int lock_init(lock* l) //
{
	l->isWriting = 0;
	l->number_of_readers = 0;
	l->exists_may_writes = 0;
	pthread_mutexattr_init(&(l->attr));
	if (pthread_mutexattr_settype(&(l->attr),PTHREAD_MUTEX_ERRORCHECK_NP)!=0)
	{
		pthread_mutexattr_destroy(&(l->attr));
		return -1;
	}
	pthread_mutexattr_settype(&(l->attr),PTHREAD_MUTEX_ERRORCHECK_NP);
	if (pthread_mutex_init(&(l->node_lock),&(l->attr))!=0)
	{
		pthread_mutexattr_destroy(&(l->attr));
		return -1;
	}
	l->arrival_queue = (queue)malloc(sizeof(struct queue_t));
	if (NULL==l->arrival_queue)
	{
		return -1;
	}
	initQueue(l->arrival_queue);
	return 0;
}

void lock_destroy(lock * l)
{
	if (l->arrival_queue!=NULL)
	{
		destroy_queue(l->arrival_queue);
		free(l->arrival_queue);
	}
	pthread_mutex_destroy(&(l->node_lock));
	pthread_mutexattr_destroy(&(l->attr));
}

void get_read_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;

	if (is_empty(l->arrival_queue,&next)==false || l->isWriting)
	{
		if (!entered_queue){
			insert(l->arrival_queue,Read,getpid());
			entered_queue=true;
		}
	}

	if (entered_queue)
	{
		pthread_mutex_unlock(&(l->node_lock));
		kill(getpid(),19);
		pthread_mutex_lock(&(l->node_lock));
	}

	l->number_of_readers++;
	pthread_mutex_unlock(&(l->node_lock));
}

void get_may_write_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;
	int currPID = getpid();

	if (is_empty(l->arrival_queue,&next)==false || l->isWriting
			|| (l->exists_may_writes))
	{
		insert(l->arrival_queue,MayWrite,currPID);
		entered_queue=true;
	}

	if (entered_queue)
	{
		pthread_mutex_unlock(&(l->node_lock));
		kill(currPID,19);
		pthread_mutex_lock(&(l->node_lock));
	}

	l->exists_may_writes = currPID;
	l->number_of_readers++;
	pthread_mutex_unlock(&(l->node_lock));
}

void get_write_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;

	if (is_empty(l->arrival_queue,&next)== false || l->isWriting
		|| (l->exists_may_writes)|| (l->number_of_readers>0))
	{
		if (!entered_queue)
		{
			insert(l->arrival_queue,Write,getpid());
			entered_queue=true;
		}
	}

	if (entered_queue){
		pthread_mutex_unlock(&(l->node_lock));
		kill(getpid(),19);
		pthread_mutex_lock(&(l->node_lock));
	}
	l->isWriting = 1;
	pthread_mutex_unlock(&(l->node_lock));
}

void upgrade_may_write_lock(lock * l)
{
	int currPID = getpid();
	pthread_mutex_lock(&(l->node_lock));

	l->number_of_readers--;

	while(l->number_of_readers>0){
		pthread_mutex_unlock(&(l->node_lock));
		kill(currPID,19);
		pthread_mutex_lock(&(l->node_lock));
	}

	l->isWriting++;
	pthread_mutex_unlock(&(l->node_lock));
}

void release_shared_lock(lock * l)
{
	pthread_mutex_lock(&(l->node_lock));
	l->number_of_readers--;


	if (getpid()==l->exists_may_writes){
		l->exists_may_writes=0;
		get_new_thread(l,l->arrival_queue);
	}
	else{

		if (l->number_of_readers==0){
			if(l->exists_may_writes){
				kill(l->exists_may_writes,18);
			}
			else{
				get_new_thread(l,l->arrival_queue);
			}
		}
	}

	pthread_mutex_unlock(&(l->node_lock));
}

void release_exclusive_lock(lock* l)
{
	pthread_mutex_lock(&(l->node_lock));
	l->isWriting--;

	if (getpid()==l->exists_may_writes){
		l->exists_may_writes=0;
	}

	Action next;
	if (is_empty(l->arrival_queue,&next) == false &&
		getpid() == l->arrival_queue->first->pid)
	{
		remove_first(l->arrival_queue);
	}

	get_new_thread(l,l->arrival_queue);
	pthread_mutex_unlock(&(l->node_lock));
}


void get_new_thread(lock *l, queue q)
{
	Action next;
	bool stop;

	if (is_empty(q, &next) == true){
		return;
	}

	if ((is_empty(q, &next) == false) && (l->number_of_readers==0))
	{
		if (Write == next)
		{
			l->isWriting = 1;

			kill(l->arrival_queue->first->pid,18);
			remove_first(q);
			return;
		}
	}

	stop = false;
	while ((is_empty(q, &next)==false) && (!stop))
	{

		if (Read == next)
		{

			kill(l->arrival_queue->first->pid,18);
			remove_first(q);
		}

		if (MayWrite == next){

			if (l->exists_may_writes == 0)
			{
				l->exists_may_writes = l->arrival_queue->first->pid;

				kill(l->arrival_queue->first->pid,18);
				remove_first(q);
			}
			else
			{
				stop=true;
			}
		}

		if (Write==next)
		{
			stop = true;
		}
	}
	return;
}


