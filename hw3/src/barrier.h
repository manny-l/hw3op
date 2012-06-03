/*
 * barrier.h
 */

#ifndef BARRIER_H_
#define BARRIER_H_

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
