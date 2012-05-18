/*
 *	lock.h
 *
 *  Created on: May 11, 2012
 *      Author: root
 */

#ifndef LOCK_H
#define LOCK_H


#include <pthread.h>


struct lock {
	int number_of_readers;
	pthread_cond_t readers_condition;
	int number_of_writers;
	pthread_cond_t writers_condition;
	pthread_mutex_t global_lock	;
};

typedef struct lock lock;

lock* lock_init();
void read_lock(lock* lock);
void read_unlock(lock* lock);
void write_lock(lock* lock);
void write_unlock(lock* lock);
void lock_destroy(lock* lock);


#endif /* LOCK_H */
