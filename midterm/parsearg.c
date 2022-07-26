#include "parsearg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char errMsg[] = "getopt - invalid command line argument\nValid example: ./program –n 3 –v 2 –c 3 –b 11 –t 3 –i inputfilepath \nn >= 2: the number of nurses (integer)\nv >= 2: the number of vaccinators (integer)\nc >= 3: the number of citizens (integer)\nb >= tc+1: size of the buffer (integer),\nt >= 1: how many times each citizen must receive the 2 shots (integer)\ni: pathname of the input file\n";

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
	

	const char* optstring = "n:v:c:b:t:i:";
	/*extract all the options entered*/
	int option = -1;
		while( (option = getopt(argc, argv, optstring)) != -1 ){
		switch(option){
			case 'n':
				flgs[nflg] = true;
				args[nflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -n argument field\n%s\n", errMsg);
				if( atoi( args[nflg] ) < 2 )
					cmdLineErr("in -n argument field\n");
				break;
			case 'v':
				flgs[vflg] = true;
				args[vflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -v argument field\n%s\n", errMsg);
				if( atoi( args[vflg] ) < 2 )
					cmdLineErr("in -v argument field\n");
				break;
			case 'c':
				flgs[cflg] = true;
				args[cflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -c argument field\n%s\n", errMsg);
				if( atoi( args[cflg] ) < 3 )
					cmdLineErr("in -n argument field\n");
				break;
			case 'b':
				flgs[bflg] = true;
				args[bflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -b argument field\n%s\n", errMsg);

				break;
			case 't':
				flgs[tflg] = true;
				args[tflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -t argument field\n%s", errMsg);
				if( atoi( args[nflg] ) < 1 )
					cmdLineErr("in -n argument field\n");
				break;
			case 'i':
				flgs[iflg] = true;
				args[iflg] = optarg;
				break;	
			case '?':
			/* *getopt* - The caller can determine that there was
          an error by testing whether the  function  return
          value  is '?'.  (By default, opterr has a nonzero
          value.)*/
  				cmdLineErr("???? %s", errMsg);
				break;
			default:
				cmdLineErr("DEFAULT %s", errMsg);
				break;
		}
	}

	if(flgs[nflg] != true || !flgs[vflg] || !flgs[cflg] || !flgs[bflg] || !flgs[tflg] || !flgs[iflg] )
		cmdLineErr("FLAGS %s", errMsg);
	//error checking calls
	// printf("Flags are: w:%d, f:%d, b:%d, t:%d, l:%d, p:%d \n", flgs[wflg], flgs[fflg], flgs[bflg], flgs[tflg], flgs[lflg], flgs[pflg]);
	// printf("Args are: w:%s, f:%s, b:%s, t:%s, l:%s, p:%s \n", args[wflg], args[fflg], args[bflg], args[tflg], args[lflg], args[pflg]);
}

bool checkIfNumeric(const char* num){
	bool numeric = true;

	int i;
	for(i = 0; numeric && num[i] != '\0'; ++i)
		if( !isdigit(num[i])  )
			numeric = false;

	return numeric;
}
