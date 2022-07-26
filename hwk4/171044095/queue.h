#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>
#include "utils.h"
#include "types.h"

typedef struct Node{
	char c;
	struct Node* next;
}Node;

void print_queue(Node* queue);
void push( char ch );
char pop();
void free_queue();
bool isQueueEmpty();
char peek();

#endif