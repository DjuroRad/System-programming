#include "queue.h"

Node* head = NULL;

// int main(){
// 	push('a');
// 	printf("%c\n", pop());	

// 	push('b');
// 	push('C');
// 	push('R');
// 	print_queue( head );

// 	printf("%c\n", pop());
	
// 	free_queue();	
// }

void print_queue(Node* queue){
	if( queue == NULL ){
		printf("\n");
		return;
	}	
	printf("%c ", queue->c);

	print_queue(queue->next);
}

Node* newNode( char new_el ){
	Node* temp = (Node*)malloc(sizeof(Node));
	if( temp == NULL )
		errExit("malloc while pushing in the queue");
	temp->c = new_el;
	temp->next = NULL;

	return temp;
}

void push( char new_el ){
	Node* new_node = newNode(new_el);

	if( head == NULL )
		head = new_node;
	else{
		Node* temp = head;
		while( temp -> next != NULL )
			temp = temp->next;//find the position at which to insert the new element
		temp -> next = new_node;
	}
}	

char pop(){
	if( head == NULL )
		errExit("Popping from an empty queue");

	char popped_val = head -> c;
	Node* to_free = head;
	head = head -> next;
	free(to_free);

	return popped_val;
}

char peek(){
	char ch;
	if( head != NULL )
		ch = head -> c;
	else
		errExit("Peeking an empty queue");
	return ch;
}

bool isQueueEmpty(){
	return head == NULL;
}

void free_queue(){

	Node* temp = head;
	while( temp != NULL ){

		temp = head;
		head = head->next;
		free(temp);
	}
	
	return;	
}