/*
 *	lock.h
 *
 *  Created on: May 11, 2012
 *      Author: root
 */

#ifndef LOCK_H
#define LOCK_H


#include <pthread.h>


struct lock
{
	int number_of_readers;
	pthread_cond_t readers_condition;
	int number_of_writers;
	pthread_cond_t writers_condition;
	int number_of_may_writers;
	pthread_cond_t may_writers_condition;
	pthread_mutex_t global_lock	;
};

typedef struct lock lock;

lock* lock_init();
void get_read_lock(lock* lock);
void release_shared_lock(lock* lock);
void get_write_lock(lock* lock);
void release_exclusive_lock(lock* lock);
void lock_destroy(lock* lock);

void get_may_write_lock(lock* l);
void upgrade_may_write_lock(lock* l);


#endif /* LOCK_H */
