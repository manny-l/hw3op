#include "lock.h"

/*----------------------------------------------------------------
functions declaration
----------------------------------------------------------------*/
void pull_from_queue(lock *l,queue q);

/*----------------------------------------------------------------
lock_init function:
Input\Output: l - the new lock
Return value: 0 - upon success, -1 otherwise
Description: allocation and initialization of a new lock
-----------------------------------------------------------------*/
int lock_init(lock* l)
{
	l->isWriting = 0;
	l->number_of_readers = 0;
	l->exists_may_writes = 0;
	pthread_mutexattr_init(&(l->attr));
	if (pthread_mutexattr_settype(&(l->attr),PTHREAD_MUTEX_ERRORCHECK_NP)!=0){
		printf("Error in pthread_mutexattr_settype\n");
		pthread_mutexattr_destroy(&(l->attr));
		return -1;
	}
	pthread_mutexattr_settype(&(l->attr),PTHREAD_MUTEX_ERRORCHECK_NP);
	if (pthread_mutex_init(&(l->node_lock),&(l->attr))!=0){
		printf("Error in pthread_mutex_init\n");
		pthread_mutexattr_destroy(&(l->attr));
		return -1;
	}
	l->arrival_queue = (queue)malloc(sizeof(struct queue_t));
	if (NULL==l->arrival_queue){
		printf("Error in memory allocation\n");
		return -1;
	}
	initQueue(l->arrival_queue);
	return 0;
}

/*----------------------------------------------------------------
lock_destroy function:
Input\Output: l - the new lock
Return value: NONE
Description: free the lock
-----------------------------------------------------------------*/
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

/*----------------------------------------------------------------
get_read_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- gets a readers lock
			- updates the number of readers on the lock
-----------------------------------------------------------------*/
void get_read_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;

	//if the process can't read enter queue
	if (is_empty(l->arrival_queue,&next)==false || l->isWriting){
		//the reader waits, thus is inserted to the queue
		if (!entered_queue){
			insert(l->arrival_queue,Read,getpid());
			entered_queue=true;
		}
	}

	//wait for the signal to run- the process waits on an empty pipe
	if (entered_queue)
	{
		char * c;
		pthread_mutex_unlock(&(l->node_lock));
		kill(getpid(),19);
		pthread_mutex_lock(&(l->node_lock));
	}

	l->number_of_readers++;
	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
get_may_write_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- gets a MW lock
			- updates the number of readers on the lock
			- updates exists_may_writes data
-----------------------------------------------------------------*/
void get_may_write_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;
	int currPID = getpid();

	//if the process can't read enter queue
	if (is_empty(l->arrival_queue,&next)==false || l->isWriting || (l->exists_may_writes)){
		//the reader waits, thus is inserted to the queue
		insert(l->arrival_queue,MayWrite,currPID);
		entered_queue=true;
	}

	//wait for the signal to run- the process waits on an empty pipe
	if (entered_queue){
		pthread_mutex_unlock(&(l->node_lock));
		kill(currPID,19);
		pthread_mutex_lock(&(l->node_lock));
	}

	l->exists_may_writes = currPID;
	l->number_of_readers++;
	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
get_write_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- gets a Write lock
			- updates isWriting data
-----------------------------------------------------------------*/
void get_write_lock(lock* l)
{
	bool entered_queue=false;
	pthread_mutex_lock(&(l->node_lock));
	Action next;

	//if the process can't write enter queue
	if (is_empty(l->arrival_queue,&next)== false || l->isWriting
		|| (l->exists_may_writes)|| (l->number_of_readers>0)){
		//the writer is inserted to the queue
		if (!entered_queue){
			insert(l->arrival_queue,Write,getpid());
			entered_queue=true;
		}
	}
	//wait for the signal to run- the process waits on an empty pipe
	if (entered_queue){
		char *c;
		pthread_mutex_unlock(&(l->node_lock));
		kill(getpid(),19);
		pthread_mutex_lock(&(l->node_lock));
	}
	l->isWriting = 1;
	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
upgrade_may_write_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- reducing the num of readers(this thread will no longer
											be counted as a reader)
			- gets a Write lock
			- updates isWriting data
-----------------------------------------------------------------*/
void upgrade_may_write_lock(lock * l)
{
	int currPID = getpid();
	pthread_mutex_lock(&(l->node_lock));
	//the MW is no longer a reader
	l->number_of_readers--;
	//waits only for readers to finish
	while(l->number_of_readers>0){
		char * c;
		pthread_mutex_unlock(&(l->node_lock));
		kill(currPID,19);
		pthread_mutex_lock(&(l->node_lock));
	}
	//after the upgrade it is no longer a reader but a writer
	l->isWriting++;
	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
release_shared_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- reducing the num of readers
			- if the releasing thread is a MW calls  pull_from_queue
			- if there are no more readers calls pull_from_queue
-----------------------------------------------------------------*/
void release_shared_lock(lock * l)
{
	pthread_mutex_lock(&(l->node_lock));
	l->number_of_readers--;

	//if it was a may write - we should check the queue.
	if (getpid()==l->exists_may_writes){
		l->exists_may_writes=0;
		pull_from_queue(l,l->arrival_queue);
	}
	else{
		//in case a the last reader is released
		if (l->number_of_readers==0){
			if(l->exists_may_writes){
				kill(l->exists_may_writes,18);
			}
			else{
				pull_from_queue(l,l->arrival_queue);
			}
		}
	}

	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
release_exclusive_lock function:
Input\Output: l - the lock
Return value: NONE
Description:- updating the isWriting field
			- calls pull_from_queue
-----------------------------------------------------------------*/
void release_exclusive_lock(lock* l)
{
	pthread_mutex_lock(&(l->node_lock));
	l->isWriting--;

	if (getpid()==l->exists_may_writes){
		l->exists_may_writes=0;
	}

	//if the current running writer is in the head of the queue
	//remove it from queue
	Action next;
	if (is_empty(l->arrival_queue,&next) == false &&
		getpid() == l->arrival_queue->first->pid){
		remove_first(l->arrival_queue);
	}

	pull_from_queue(l,l->arrival_queue);
	pthread_mutex_unlock(&(l->node_lock));
}

/*----------------------------------------------------------------
pull_from_queue function:
Input: 		  q - the queue on the current lock
Input\Output: l - the lock
Return value: NONE
Description:- pulls threads on the lock queue until reaching
			  a Writer or a second MW.
-----------------------------------------------------------------*/
void pull_from_queue(lock *l, queue q)
{
	Action next;
	bool stop;

	//end if queue is empty
	if (is_empty(q, &next) == true){
		return;
	}


	//if the first node is a writer, and there are no reader wake up the writer
	if ((is_empty(q, &next) == false) && (l->number_of_readers==0))
	{
		if (Write == next)
		{
			l->isWriting = 1;
			//wake up the relevant writer, 18=SIGCONT
			kill(l->arrival_queue->first->pid,18);
			remove_first(q);
			return; //the first one in the queue is writer, if it runs no one else can run.
		}
	}

	//wake up all the readers until the next writer
	stop = false;
	while ((is_empty(q, &next)==false) && (!stop))
	{
		//letting the readers run
		if (Read == next)
		{
			//wake up the relevant reader, 18=SIGCONT
			kill(l->arrival_queue->first->pid,18);
			remove_first(q);
		}

		if (MayWrite == next){
			//no more then 1 MW can be waked up. stop when reaching the second MW
			if (l->exists_may_writes == 0)
			{
				l->exists_may_writes = l->arrival_queue->first->pid;
				//wake up the relevant MW, 18=SIGCONT
				kill(l->arrival_queue->first->pid,18);
				remove_first(q);
			}
			else{
				stop=true;
			}
		}

		if (Write==next){
			stop = true;
		}
	}
	return;
}


