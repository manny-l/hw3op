/*
 * barrier.c
 *
 *  Created on: May 18, 2012
 *      Author: root
 */

#include <pthread.h>
#include <stdlib.h>
#include "barrier.h"


barrier_t* init_barrier(int N)
{
	if (N<=0)
	{
		return NULL;
	}

	barrier_t* barrier = (barrier_t*)malloc(sizeof(barrier_t));

	if (!barrier)
	{
		return NULL;
	}

	barrier->num = N;
	pthread_mutex_init(&barrier->Mlock, NULL);
	pthread_cond_init(&barrier->condition, NULL);
	return barrier;
}

/*********************************************************************************/

void destroy_barrier(barrier_t* bar)
{
	if (bar==NULL)
	{
		return;
	}
	pthread_mutex_destroy(&bar->Mlock);
	pthread_cond_destroy(&bar->condition);
	free(bar);
	return;
}

/***********************************************************************************/

int barrier(barrier_t* bar)
{
	if (bar==NULL)
	{
		return -1;
	}

	pthread_mutex_lock(&bar->Mlock);

	if (bar->num==0)
	{
		pthread_mutex_unlock(&bar->Mlock);
		return -1;
	}

	bar->num--;

	if (bar->num==0)
	{
		pthread_cond_broadcast(&(bar->condition));
	}
	else
	{
		while (bar->num > 0)
		{
			pthread_cond_wait(&(bar->condition),&bar->Mlock);
		}

	}

	pthread_mutex_unlock(&bar->Mlock);

	return 0;
}
