#include "utils.h"

void exErr(const char* message){
	fprintf(stderr, "Error message is: %s, Error number is: %d\n", message,errno);
	exit(EXIT_FAILURE);
}