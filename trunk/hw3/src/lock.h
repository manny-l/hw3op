#ifndef _lock_H
#define _lock_H

#include <pthread.h>
#include <stdio.h>
#include "myQueue.h"

struct lock_t {
	pthread_mutex_t node_lock;
	pthread_mutexattr_t attr;
	queue arrival_queue;
	int isWriting;
	int number_of_readers;
	int exists_may_writes;
};

typedef struct lock_t lock;

int lock_init(lock * l);
void lock_destroy(lock * l);
void get_read_lock(lock * l);
void get_may_write_lock(lock * l);
void get_write_lock(lock * l);
void upgrade_may_write_lock(lock * l);
void release_shared_lock(lock * l);
void release_exclusive_lock(lock * l);

#endif

