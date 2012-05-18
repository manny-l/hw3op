
#include "lock.h"



lock* lock_init()
{
	lock* rw_lock = (lock*)malloc(sizeof(lock));
	if(rw_lock == NULL)
	{
		return NULL;
	}
	rw_lock->number_of_readers = 0;
	pthread_cond_init(&(rw_lock->readers_condition),NULL);
	rw_lock->number_of_writers = 0;
	pthread_cond_init(&rw_lock->writers_condition,NULL);
	pthread_mutex_init(&rw_lock->global_lock,NULL);
	return rw_lock;

}
/***********************************************************************/


void read_lock (lock* rw_lock)
{
	pthread_mutex_lock(&rw_lock->global_lock);
	while (rw_lock->number_of_writers > 0)
	{
		pthread_cond_wait (&rw_lock->readers_condition, &rw_lock->global_lock);
	}
	rw_lock->number_of_readers++;
	pthread_mutex_unlock (&rw_lock->global_lock);

}

/***********************************************************************/


void read_unlock (lock* rw_lock)
{
	pthread_mutex_lock (&rw_lock->global_lock);
	rw_lock->number_of_readers--;
	if (rw_lock->number_of_readers == 0)
	{
		pthread_cond_signal(&rw_lock->writers_condition);
	}
	pthread_mutex_unlock(&rw_lock->global_lock);
}


/***********************************************************************/

void write_lock (lock* rw_lock)
{
	pthread_mutex_lock (&rw_lock->global_lock);
//	if ((rw_lock->number_of_writers)>0)
//	{
//		printf("number of writers %d\n",rw_lock->number_of_writers);
//	}
	while ((rw_lock->number_of_writers > 0) || (rw_lock->number_of_readers >0)){
//		if ((rw_lock->number_of_writers)<=0)
//		{
//			printf("number of writers la %d\n",rw_lock->number_of_writers);
//		}
		pthread_cond_wait (&rw_lock->writers_condition, &rw_lock->global_lock);
	}

		rw_lock->number_of_writers++;
//	if (rw_lock->number_of_writers == 1)
//	{
//		printf("raised here\n");
//	}
	pthread_mutex_unlock(&rw_lock->global_lock);
}

/***********************************************************************/

void write_unlock(lock* rw_lock)
{
	pthread_mutex_lock (&rw_lock->global_lock);
	rw_lock->number_of_writers--;
//	printf("entered\n");
	if(rw_lock->number_of_writers == 0)
	{
//		printf("broadcasted\n");
		pthread_cond_broadcast(&rw_lock->readers_condition);
		pthread_cond_signal (&rw_lock->writers_condition);
	}
	pthread_mutex_unlock (&rw_lock->global_lock);

}

/***********************************************************************/

void lock_destroy(lock* lock)
{
	pthread_cond_destroy (&lock->readers_condition);
	pthread_cond_destroy (&lock->writers_condition);
	pthread_mutex_destroy(&lock->global_lock);
	free(lock);
	return;
}





