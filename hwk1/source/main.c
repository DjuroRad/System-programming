#include "parsearg.h"
#include "search.h"
#include "types.h"
#include <signal.h>

void handler(int sig){
	fprintf(stderr, "SIGINT signal sent, deallocating memory and exiting!\n");
	free_allocated();
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
	struct sigaction sact;
	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = handler;

	if( sigaction(SIGINT, &sact, NULL) == -1 ){
		fprintf(stderr, "Failed to install SIGING signal handler\n");
	}

	parseArgs(argc, argv);

	searchDirRec(args[wflg]);

	fprintf(stdout, "%s", formatted_text);

	free_allocated();

	return 0;
}	