#ifndef _queue_H
#define _queue_H

#include <stdlib.h>

typedef enum {Read,Write,MayWrite} Action;
typedef enum {false,true} bool;

typedef struct queue_data{
	Action type;
	struct queue_data * next;
	int pid;
}* data;


typedef struct queue_t{
	data first;
	data last;
} * queue;

//function declaration
void initQueue(queue q);
void insert(queue q, Action type,int pid);
bool remove_first(queue q);
void destroy_queue(queue q);
bool is_empty(queue q, Action* type);
void print_queue(queue q);

#endif

