#include "parsearg.h"
#include "utils.h"
#include <string.h>
#define BUFF_SIZE 4096

int sockfd_client = -1;
char** queries = NULL;/*dynamically allocated query that needs to be sent*/
char* response = NULL;
int queries_n = 0;
int query_fd = -1;


char* curr_row = NULL;
/*read row will return false when it reaches the end of file*/
bool readRow();
void clearRow();
bool checkID();
void establishConnection();
void handler();
void clear();
void setupSignalHandler();
void runClient();
void readQueries();
void addToQuerries();/*adding from current row of course*/

int main( int argc, char** argv ){

	parseArgs( argc, argv );

	setupSignalHandler();

	readQueries();
	
	// printf("Printing queries that are in the memor %d\n", queries_n);
	// for(int i=0; i<queries_n; ++i)
	// 	printf("%s\n", queries[i]);
	// clear();
	// exit(1);
	printf("Client-%s connecting to %s\n", Args[id_flg], Args[address_flg]);
	establishConnection();

	/*for some reason this won't work when i call it in runClient like here*/
	// runClient();

	clear();

	return 0;
}
void readQueries(){
	query_fd = open_util( Args[path_flg], O_RDONLY);

	while( readRow() ){
		printf("%s\n", curr_row);
		if(checkID())
			addToQuerries();/*adding from current row of course*/
		clearRow();
	}
	/*here is the last row that was read and it also needs to be inspected and added to the queries if needed!*/
}

void addToQuerries(){/*adding from current row of course*/
	/*iterate to the place where the command only is found and ID is omitted*/
	int i=0;
	while( curr_row[i] != ' ' )
		++i;
	++i;/*now it is at the right place*/

	printf("\n\n\n%s\n", curr_row);
	if( queries_n == 0 )
		queries = (char**)malloc_util(sizeof(char*));
	else
		queries = (char**)realloc_util( queries, (queries_n+1)*sizeof(char*));

	int len = strlen(curr_row)-i+1;
	queries[queries_n++] = (char*)calloc_util(len, sizeof(char));

	strcpy(queries[queries_n-1], curr_row+i);

	// strcpy(queries[queries_n-1], curr_row+i);
	// int j;
	// for( j = 0; query[j] != '\0'; ++j){
	// 	queries[queries_n-1][j] = query[j];
	// 	printf("query %d - %c\n", j, query[j]);
	// }
}

bool readRow(){
	if(curr_row == NULL)
		curr_row = (char*)calloc_util(1, sizeof(char));
	
	int row_i = 0;
	char new_char;
	int ret_val = 1;
	do{
		
		ret_val = read_util( query_fd, &new_char, sizeof(char) );

		if( ret_val && new_char != '\n'){
			curr_row[row_i++] = new_char;
		}
		else
			curr_row[row_i++] = '\0';

		curr_row = (char*)realloc_util(curr_row, (row_i+1)*sizeof(char));
		curr_row[row_i] = '\0';
	}while( ret_val && new_char != '\n');

	return ret_val;
}
bool checkID(){
	/*using the row we are currently examining*/
	char str_to_check[ strlen(curr_row) ];
	strcpy(str_to_check, curr_row);
	char* id = strtok(str_to_check, " ");
	if( strcmp( id, Args[id_flg] ) == 0 )
		return true;
	return false;
}
void runClient(){

	char msg[BUFF_SIZE] = {0};
	int i;
	int num_records = -1;
	for( i = 0; i<queries_n; ++i ){
		
		for(int j = 0; queries[i][j] != '\0'; ++j)
			msg[j] = queries[i][j];
		strcpy(msg, queries[i]);
		printf("Client-%s connected and sending query \'%s\'", Args[id_flg], msg);
		// write(sockfd_client, msg, sizeof(msg));/*send the query to be executed*/
		send(sockfd_client, queries[i], strlen(queries[i]), 0);
		exit(1);
		clock_t begin = clock();	
		/*read the response here*/
		/*will have to read it dynamically ---> reallocations!*/
		/* here, do your time-consuming job / reading from the server*/

		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Server's response to client is %d records, and arrived in %.1lf seconds", num_records, time_spent );
	}
	// while( 1 ){
	// 	/*get the query from the input file*/
	// 	char query_dummy;
	// 	query_dummy = getchar();
	// 	/*send the query to the server!!!*/
	// 	write(sockfd_client, &query_dummy, sizeof(query_dummy));

	// 	/*now wait for a response from the database/server*/
	// 	// free(query);
	// 	// query = NULL;
	// }

	printf("A total of %d queires were executed. Client is terminating.\n", queries_n);
}
void setupSignalHandler(){
	struct sigaction sact;
	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = handler;
	sigaction_util(SIGINT, &sact, NULL);
}
void establishConnection(){
	int sockfd_client;
	SA servaddr;

	sockfd_client = socket_util(AF_INET, SOCK_STREAM, 0);

	// /bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	// servaddr.sin_addr.s_addr = inet_addr(Args[address_flg]);
	servaddr.sin_port = htons(atoi(Args[port_flg_cli]));

	// Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, Args[address_flg], &servaddr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        errExit("inet_pton");
    }
	// inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	connect_util( sockfd_client, (struct sockaddr*)&servaddr, sizeof(servaddr) );




	/*client run needs to be pasted in here*/



	// char msg[strlen(queries[0])];
	// strcpy(msg, queries[0]);
	// int i;
	// for( i = 0; i<queries_n; ++i ){
	// 	char msg[BUFF_SIZE]={0};
	// 	strcpy(msg, queries[i]);
	// 	write(sockfd_client, msg, BUFF_SIZE);
	// }

	char msg[BUFF_SIZE] = {0};
	int i;
	int num_records = -1;
	for( i = 0; i<queries_n; ++i ){
		strcpy(msg, queries[i]);
		printf("Client-%s connected and sending query \'%s\'\n", Args[id_flg], msg);
		// write(sockfd_client, msg, sizeof(msg));/*send the query to be executed*/
		write(sockfd_client, msg, BUFF_SIZE);
		clock_t begin = clock();
		bzero(msg, BUFF_SIZE);	
		/*read the response here*/
		/*will have to read it dynamically ---> reallocations!*/
		/* here, do your time-consuming job / reading from the server*/
		int ret_val = 1;
		do{
			ret_val = read(sockfd_client, msg, BUFF_SIZE);
			printf("Server's response is %s\n", msg);
		}while(ret_val>=BUFF_SIZE);
		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Server's response to client is %d records, and arrived in %.1lf seconds\n", num_records, time_spent );
	}

	printf("A total of %d queires were executed. Client is terminating.\n", queries_n);
}
void clear(){
	if( sockfd_client != -1 )
		close_util(sockfd_client);
	if( queries != NULL ){
		for(int i =0; i<queries_n; ++i ){
			free(queries[i]);
			queries[i] = NULL;
		}

		free(queries);
		queries = NULL;
	}

	if( query_fd == -1 )
		close_util(query_fd);

	if( curr_row != NULL ){
		free(curr_row);
		curr_row = NULL;
	}
	if(response!=NULL){
		free(response);
		response = NULL;
	}

	return;
}

void clearRow(){
	if( curr_row != NULL )
		free(curr_row);
	curr_row = NULL;

	return;
}
void handler(){
	clear();
	printf("Program interrupted\n");
	exit(1);
}