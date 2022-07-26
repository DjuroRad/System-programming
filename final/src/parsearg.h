#ifndef PARSEARG_H
#define PARSEARG_H

#ifdef __GNUC__
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

#include <stdarg.h>
#include <ctype.h>
#include "types.h"
#define FLG_N 4

bool flgs[FLG_N];
char* Args[FLG_N];

void cmdLineErr(const char* format, ...) NORETURN;
void parseArgs(int argc, char* argv[]);
bool checkIfNumeric(const char* num);
bool isValidPermissionIn(const char* permissions);
/*this function is for error checking DELETE IT LATER DJURO!!!!*/
// void errCheck(bool flgs[], char* warg, char* farg, char* barg, char* targ, char* larg, char* parg);

#endif