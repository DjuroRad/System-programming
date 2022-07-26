#include "parsearg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cmdLineErr(const char* format, ...){
	va_list argList;
	fflush(stdout);

	fprintf(stderr, "Command-line usage error\n");
	va_start(argList, format);
	vfprintf(stderr, format, argList);
	va_end(argList);

	fflush(stderr);
	exit(EXIT_FAILURE);
}

void parseArgs(int argc, char* argv[]){
	/*set flags to false*/
	if( memset(flgs, false, FLG_N*sizeof(flg) ) == NULL ){
		fprintf(stderr,"memset failed to set the memory");
		exit(EXIT_FAILURE);
	}
	

	const char* optstring = "b:s:f:m:";
	/*extract all the options entered*/
	int option = -1;
		
		while( (option = getopt(argc, argv, optstring)) != -1 ){
			
			switch(option){
				case 'b':
					flgs[bflg] = true;
					args[bflg] = optarg;
					if( !checkIfNumeric(optarg) )
						cmdLineErr("-b flag is not numeric");
					break;
				case 's':
					flgs[sflg] = true;
					args[sflg] = optarg;
					break;
				case 'f':
					flgs[fflg] = true;
					args[fflg] = optarg;
					break;	
				case 'm':
					flgs[mflg] = true;
					args[mflg] = optarg;
					break;	
				case '?':
				/* *getopt* - The caller can determine that there was
	          an error by testing whether the  function  return
	          value  is '?'.  (By default, opterr has a nonzero
	          value.)*/
	  				cmdLineErr("???? getopt - invalid command line argument, unknown option:%c\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n", optopt);
					break;
				default:
					fprintf(stderr, "\n\n\nIN DEFAULT\n\n\n");
					cmdLineErr("DEFAULT getopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");
					break;
			}
	}

	if( flgs[bflg] != true || flgs[sflg] != true || flgs[fflg] != true || flgs[mflg] != true )
		cmdLineErr("Please enter all 4 flags in order to proceed.\n-b: number of potatos ( > 0 )\n-s: name of shared memory\n-f: file with fifo names\n-m: semaphore name\n");

}

bool checkIfNumeric(const char* num){
	bool numeric = true;

	/*Check if the first digit is larger than 0*/
	if(num[0] < '0')
		numeric = false;

	int i;
	for(i = 0; numeric && num[i] != '\0'; ++i)
		if( !isdigit(num[i])  )
			numeric = false;

	return numeric;
}
