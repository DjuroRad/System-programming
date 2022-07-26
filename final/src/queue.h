#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>
#include "utils.h"
#include "types.h"

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

typedef struct Node{
	int fd_curr;
	struct Node* next;
}Node;

void print_queue(Node* queue);
void push( int new_fd );
char pop();
void free_queue();
bool isQueueEmpty();
char peek();

#endif