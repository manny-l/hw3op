#include <stdlib.h>
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

	rw_lock->number_of_may_writers = 0;
	pthread_cond_init(&rw_lock->may_writers_condition,NULL);

	pthread_mutex_init(&rw_lock->global_lock,NULL);
	return rw_lock;

}
/***********************************************************************/


void get_read_lock (lock* l)
{
	pthread_mutex_lock(&l->global_lock);
	while (l->number_of_writers > 0 || l->number_of_may_writers > 1)
	{
		pthread_cond_wait (&l->readers_condition, &l->global_lock);
	}
	l->number_of_readers++;
	pthread_mutex_unlock (&l->global_lock);

}

/***********************************************************************/


void release_shared_lock (lock* l)
{
	pthread_mutex_lock (&l->global_lock);
	l->number_of_readers--;
	if (l->number_of_readers == 0)
	{
		pthread_cond_signal(&l->writers_condition);
	}
	pthread_mutex_unlock(&l->global_lock);
}


/***********************************************************************/

void get_write_lock (lock* l)
{
	pthread_mutex_lock (&l->global_lock);
//	if ((rw_lock->number_of_writers)>0)
//	{
//		printf("number of writers %d\n",rw_lock->number_of_writers);
//	}
	while ((l->number_of_writers > 0) || (l->number_of_readers >0))
	{
//		if ((rw_lock->number_of_writers)<=0)
//		{
//			printf("number of writers la %d\n",rw_lock->number_of_writers);
//		}
		pthread_cond_wait (&l->writers_condition, &l->global_lock);
	}

		l->number_of_writers++;
//	if (rw_lock->number_of_writers == 1)
//	{
//		printf("raised here\n");
//	}
	pthread_mutex_unlock(&l->global_lock);
}

/***********************************************************************/

void release_exclusive_lock(lock* l)
{
	pthread_mutex_lock (&l->global_lock);
	l->number_of_writers--;
//	printf("entered\n");
	if(l->number_of_writers == 0)
	{
//		printf("broadcasted\n");
		pthread_cond_broadcast(&l->readers_condition);
		pthread_cond_signal (&l->writers_condition);
	}
	pthread_mutex_unlock (&l->global_lock);
}

/***********************************************************************/

void lock_destroy(lock* l)
{
	pthread_cond_destroy (&l->readers_condition);
	pthread_cond_destroy (&l->writers_condition);
	pthread_cond_destroy (&l->may_writers_condition);
	pthread_mutex_destroy(&l->global_lock);
	free(l);
	return;
}


/***********************************************************************/

void get_may_write_lock(lock* l)
{
	/* Get lock in the may-write mode: Allowed if there are just readers in
	 * the lock, and no additional may-writers. Also there should be no
	 * writers or updater already waiting. We allow only one may-writer at
	 * a time because we do not want to deal with two or more simultaneous
	 * upgrade requests.
	 */

	pthread_mutex_lock (&l->global_lock);
//	if ((rw_lock->number_of_writers)>0)
//	{
//		printf("number of writers %d\n",rw_lock->number_of_writers);
//	}
	while ((l->number_of_writers > 0) || (l->number_of_may_writers >1))
	{
//		if ((rw_lock->number_of_writers)<=0)
//		{
//			printf("number of writers la %d\n",rw_lock->number_of_writers);
//		}
		pthread_cond_wait (&l->may_writers_condition, &l->global_lock);
	}

	l->number_of_may_writers++;
//	if (rw_lock->number_of_may_writers == 1)
//	{
//		printf("raised here\n");
//	}
	pthread_mutex_unlock(&l->global_lock);

}


void upgrade_may_write_lock(lock* l)
{
	/* Upgrade lock in may-write mode: Notice that this is relevant only
	 * if the lock is in MayWrite mode. You may use some assert here.
	 * There is no upgrade for either readers or writers! Also notice that
	 * this upgrade has a priority over any waiting writer! Any waiting
	 * writer will have to continue to wait till this may-writer
	 * (that is turning to be writer) will receive an upgrade, and be done.
	 * The thread waiting for an upgrade will, in turn, need to wait till
	 * all current readers are done. The upgraded thread becomes a full
	 * writer for all purposes (no implications from formerly being a
	 * MayWriter).
	 */

	return;
}
