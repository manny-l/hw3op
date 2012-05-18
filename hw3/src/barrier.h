/*
 * barrier.h
 *
 *  Created on: May 18, 2012
 *      Author: root
 */

#ifndef BARRIER_H_
#define BARRIER_H_

/*
struct context_t
{
	//hashtable* hash;
	int hash;
};

typedef struct context_t context_t;
*/

struct barrier_t
{
	pthread_mutex_t Mlock;
	int num;
	pthread_cond_t condition;
};

typedef struct barrier_t barrier_t;

barrier_t* init_barrier(int N);
void destroy_barrier(barrier_t* bar);
int barrier(barrier_t* bar);

#endif /* BARRIER_H_ */
