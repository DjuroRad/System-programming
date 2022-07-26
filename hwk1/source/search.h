#ifndef SEARCH_H
#define SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
	
#include "search.h"	
#include "format_string.h"
#include "types.h"
#include "parsearg.h"

bool searchDirRec2(const char* dirpathname, unsigned int step);
void searchDirRec(const char* dirpathname);
bool checkFile( struct dirent* sd, const char* dirpathname);
void free_allocated();
char* to_open;

#endif