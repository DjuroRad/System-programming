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
	

	const char* optstring = "w:f:b:t:l:p:";
	/*extract all the options entered*/
	int option = -1;
		while( (option = getopt(argc, argv, optstring)) != -1 ){
		switch(option){
			case 'w':
				flgs[wflg] = true;
				args[wflg] = optarg;
				break;
			case 'f':
				flgs[fflg] = true;
				args[fflg] = optarg;
				break;
			case 'b':
				flgs[bflg] = true;
				args[bflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -b argument field\ngetopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");

				break;
			case 't':
				flgs[tflg] = true;
				args[tflg] = optarg;
				if(strlen(optarg) != 1)
					cmdLineErr("in -b argument field\ngetopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");
				switch(optarg[0]){
					case 'd':case 's':case 'b':case 'c':case 'f':case 'p':case 'l': break;
					default:
						cmdLineErr("in -t argument field\ngetopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");
						break;
				}
				break;
			case 'l':
				flgs[lflg] = true;
				args[lflg] = optarg;
				if( !checkIfNumeric(optarg) )
					cmdLineErr("in -l argument field\ngetopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");

				break;
			case 'p':
				flgs[pflg] = true;
				args[pflg] = optarg;
				// printf("is valid is %d\n", isValidPermissionIn(optarg));
				if( strlen(optarg)!=9 || !isValidPermissionIn(optarg) )
					cmdLineErr("in -p argument field\ngetopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");
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

	if(flgs[wflg] != true || argc < 3)
		cmdLineErr("FLAGS getopt - invalid command line argument\nValid example: ./myFind -w targetDirectoryPath -f ‘lost+file‘ -b 100 -t b \n-w : directory path is mandatory\n-f : filename (case insensitive), supporting the following regular expression: + \n-b : file size (in bytes)\n-t : file type (d: directory, s: socket, b: block device, c: character device f: regular file, p: pipe, l: symbolic link)\n-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n-l: number of links\n");
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
