#include "parsearg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
const char* msg = "Form of the cmd input should be:\n\t./server -p PORT -o pathToLogFile –l poolSize –d datasetPath\n";
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
	

	const char* optstring = "p:o:l:d:";
	/*extract all the options entered*/
	int option = -1;
		while( (option = getopt(argc, argv, optstring)) != -1 ){
		
		switch(option){
			case 'p':
				flgs[port_flg] = true;
				Args[port_flg] = optarg;
				break;
			case 'o':
				flgs[logfile_flg] = true;
				Args[logfile_flg] = optarg;
				break;
			case 'l':
				flgs[poolSize_flg] = true;
				Args[poolSize_flg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("%s", msg);

				break;
			case 'd':
				flgs[dataset_flg] = true;
				Args[dataset_flg] = optarg;
				break;
			case '?':
			/* *getopt* - The caller can determine that there was
          an error by testing whether the  function  return
          value  is '?'.  (By default, opterr has a nonzero
          value.)*/
  				cmdLineErr("%s", msg);
				break;
			default:
				cmdLineErr("%s", msg);
				break;
		}
	}

	if( argc < 9 )
		cmdLineErr("%s", msg);
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

	if( strlen(num) == 1 && num[0] - '0' < 2 )
		numeric = false;
	return numeric;
}

bool isValidPermissionIn(const char* permissions){
	int i;
	bool valid = true;

	for(i=0;valid && i<9;++i){
		// printf("%d\n", i);
		if( (i%3==0 && !(permissions[i] == 'r' || permissions[i] == '-') ) ) valid = false;
		if( (i%3==1 && !(permissions[i] == 'w' || permissions[i] == '-') ) ) valid = false;
		if( (i%3==2 && !(permissions[i] == 'x' || permissions[i] == '-') ) ) valid = false;
	}
	return valid;
}
//function for error checking
// void errCheck(bool flgs[], char* warg, char* farg, char* barg, char* targ, char* larg, char* parg){
// 	// printf("Flags are: w:%d, f:%d, b:%d, t:%d, l:%d, p:%d \n", flgs[wflg], flgs[fflg], flgs[bflg], flgs[tflg], flgs[lflg], flgs[pflg]);
// 	// if(flgs[wflg] == true ) printf("WARG: %s\n", warg);
// 	// if(flgs[fflg] == true ) printf("FARG: %s\n", farg);
// 	// if(flgs[bflg] == true ) printf("BARG: %s\n", barg);
// 	// if(flgs[tflg] == true ) printf("TARG: %s\n", targ);
// 	// if(flgs[lflg] == true ) printf("LARG: %s\n", larg);
// 	// if(flgs[pflg] == true ) printf("PARG: %s\n", parg);		
// }
