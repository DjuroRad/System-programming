#ifndef FORMAT_STRING_H
#define FORMAT_STRING_H

#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *formatted_text;
char** newpaths;
int np;

void add_to_text(const char* to_append, bool is_dir, int start, int step);
void print_formatted(char* d_name, unsigned int step);
char* getNewPath(const char* dirpathname,const char* d_name);
bool cmpChars(char c1, char c2);

#endif