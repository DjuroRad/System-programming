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
	if( argc != 4 )
		cmdLineErr("not 4 arguments are inserted\n");
	args[hwk_flg] = argv[1];
	args[student_flg] = argv[2];
	args[cash_flg] = argv[3];
	if( !checkIfNumeric( args[cash_flg] ) )
		cmdLineErr("3rd cmd argument has to be numeric!\n");
}	

bool checkIfNumeric(const char* num){
	bool numeric = true;

	int i;
	for(i = 0; numeric && num[i] != '\0'; ++i)
		if( !isdigit(num[i])  )
			numeric = false;

	return numeric;
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
