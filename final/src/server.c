#include <string.h>
#include "utils.h"
#include "parsearg.h"
#include "queue.h"
#include "databaseManipulation.h"
#define BUFF_SIZE 4096
/*server and a client*/
/*open a stream socket and wait for clients to connect and conduct SQL queries*/

/*
	Server - single instance

	1. open stream socket for clients
	2. conduct SQL queries

	NOTE: Server is a deamon
		1.Avoid double instantiation ( it should not be possible to start 2 instances of the server process )
		2. make sure the server process has no controlling terminal
		3. close all of its inherited open files
*/

/*
	Client - multiple instances
	
	1.read SQL queries from a file
	2. send to server
	3. print received responses
*/
typedef struct NodeT{
	char line[BUFF_SIZE];
}NodeT;
void add( const char* line );/*add to this array*/

typedef struct Thread{
	pthread_t thread;
	int index;
	bool available;
}Thread;

typedef struct Task{
	/*incoming query shold be placed in this structure!!!*/

	/*i need a queue of Tasks!!!*/
	/*need a mutex for the queue i am using*/
	/*conditional variable so that we could wait for the queue to get a new query*/
}Task;

NodeT dataset[10000];
int dataset_count = 0;

int csv_fd = -1;
FILE* csv_p = NULL;

int server_sock_fd = -1;
int client_socket_fd = -1;

int THREAD_N = 0;
/*
	reader  writer implementation for the thread pool which 
	will write or read according to the query it is processing at the moment
*/
int AR = 0;
int AW = 0;
int WR = 0;
int WW = 0;

pthread_cond_t okToRead;
pthread_cond_t okToWrite;
pthread_mutex_t response_db_mutex;

pthread_mutex_t m;
/*end of reader-writer needed variables*/


// void readCSV( char* filename );
void clear();
void init(Thread thread_pool[]);
void server_run();
bool server_process_query(int client_fd);

/*method to be executed by a thread pool's thread*/
void* process_query(void* arg);

void printMsg(const char* fmt, ...);/*will be put into a log file later on*/

char** Reader(const char* query);
char** Writer(const char* query);

int main( int argc, char** argv ){

	struct sigaction sact;
	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = clear;
	sigaction_util(SIGINT, &sact, NULL);
	
	/*this the servers main thread*/
	parseArgs(argc, argv);
	Thread th[THREAD_N];

	printMsg("Loading dataset...\n");
	readCSV(Args[dataset_flg]);
	printMsg("Dataset loaded in %.2lf seconds with %d records\n", getExecutionTime(), getRowCount());
	init(th);
	


	printf("THEY ALL JOINED AFTER THIS I SUPPOSE OR AS PER SE THEY SHOULD HAVE EXITED!\n");

	printf("Got in here");
	server_run();
	// int i;
	// for( i = 0; i<dataset_count; ++i )
	// 	printf("%s\n", dataset[i].line);

	/*create a stream socket and wait for connection from the client!*/

	return 0;
}

// void readCSV( char* filename ){
// 	csv_fd = open_util( filename, O_RDONLY);
// 	csv_p = fdopen( csv_fd, "r");

// 	char line[BUFF_SIZE];
// 	while( fgets(line, BUFF_SIZE, csv_p) ){
// 		add( line );it will add a new line to the array of our nodes that are just line by line read this csv text and stored the database's table
// 	}


// 	close_util( csv_fd );
// 	csv_fd = -1;
// 	csv_p = NULL;
		
// }

void clear(){

	if( csv_fd != -1 ){
		close_util(csv_fd);
		fclose_util( csv_p );
	}

	pthread_mutex_destroy(&mutexQueue);
    pthread_cond_destroy(&condQueue);

	pthread_mutex_destroy(&m);
    pthread_cond_destroy(&okToRead);
    pthread_cond_destroy(&okToWrite);
    pthread_mutex_destroy(&response_db_mutex);

    free_queue();
}

void init( Thread thread_pool[] ){
	THREAD_N = atoi(Args[poolSize_flg]);
	// readCSV( Args[dataset_flg] );
	pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);

    pthread_mutex_init( &m, NULL );
    pthread_mutex_init( &response_db_mutex, NULL );
    pthread_cond_init( &okToRead, NULL );
	pthread_cond_init( &okToWrite, NULL );

	pthread_attr_t attribute;
	pthread_attr_init(&attribute);
	pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);

	int i;
	printMsg("A pool of %d threads has been created\n", THREAD_N);
	for( i = 0; i<THREAD_N; ++i ){
		thread_pool[i].available = true;
		thread_pool[i].index = i;
		pthread_create_util( &thread_pool[i].thread, &attribute, &process_query, &thread_pool[i]);
	}

	server_run();/*connect with the client here NOW and get the queries that need to be processed*/
	
	// sleep(5);
	for( int i = 0; i<THREAD_N; ++i )
		pthread_join_util(thread_pool[i].thread, NULL);

	return;
}

void server_run(){
	SA servaddr, cli;
	/*create the socket and connect to the client!*/
	server_sock_fd = socket_util( AF_INET, SOCK_STREAM, 0 );
	// printf("Socket is created\n");
	int valread;

	int opt=1;
	if (setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
	
	
	/*client_fd is declared as global so that we can close it in the case an error occurs*/

	bzero(&servaddr,sizeof(servaddr));

	servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(Args[port_flg]));
	/*server.sin_addr.s_addr = inet_addr("127.0.0.1");*/

	bind_util( server_sock_fd, (struct sockaddr*) (&servaddr), sizeof(servaddr) );
	/*listen for conncetions*/
	if( listen(server_sock_fd, 5) != 0 )
		errExit("listen");

	/*now we can run it properly*/
	// bool client_quit = false;
	do{
		int client_name_len = sizeof(cli);
		// socklen_t client_name_len = sizeof(cli);
		client_socket_fd = accept_util( server_sock_fd, (struct sockaddr*)&cli, (socklen_t*)&client_name_len );
		/*now add this file descriptor to the queue of ours*/
		pthread_mutex_lock(&mutexQueue);
			push(client_socket_fd);
		pthread_mutex_unlock(&mutexQueue);
		pthread_cond_signal(&condQueue);/*now signal an available thread to execute this!!!*/

		char buff[BUFF_SIZE];
		bzero(buff, sizeof(buff));
		
		do{
			valread = read(client_socket_fd, buff, sizeof(buff));
			if( valread )
				printMsg("\nMessage arrived from client %s\n", buff);

			// write(client_socket_fd, "HELLO", 5);

		}while(valread);
		exit(1);
		/*handle the connection*/
		
		// client_quit = server_process_query(client_socket_fd);
		// close_util( client_socket_fd );
		// client_socket_fd = -1;


	}while( 1 );

	printf("All queries have been processed\n");
	exit(1);
	close_util( server_sock_fd );
	
}	
bool server_process_query(int client_fd){
	printf("server is processing client's query\n");
	/*read the contet inside this client's fd!*/
	char msg[BUFF_SIZE];
	bzero(msg, BUFF_SIZE);
	if( read_util(client_fd, msg, sizeof(msg)) == 0 )
		printf("READ FAILED\n"); 
	printf("\n\n\nMESSAGE FROM CLIENT IS and %c - %s\n\n\n",msg[0], msg);
	sleep(3);
	pthread_mutex_lock(&mutexQueue);
	push('w');
	pthread_mutex_unlock(&mutexQueue);
	pthread_cond_signal(&condQueue);
	/*Chose a thread from the thread pool or wait until at least one thread is free to process this!*/

	return false;
}

void printMsg(const char* fmt, ...){
	va_list args;
	va_start(args, fmt);

	while (*fmt != '\0') {
		if( *fmt == '%' ){
			++fmt;
			if( *fmt == 'c' ){
				char dummy = va_arg(args, int );
				printf("%c", dummy);/*change to fprintf to the log file for all the things that are used in here!*/
			}
			else if( *fmt == 's' ){
				const char* str = va_arg(args, char* );
				printf("%s", str);/*change to fprintf to the log file for all the things that are used in here!*/
			}
			else if( *fmt == 'd' ){
				int integer = va_arg( args, int );
				printf("%d", integer);/*change to fprintf to the log file for all the things that are used in here!*/
			}
			else if( *fmt == 'f' ){
				float number = va_arg(args, double);
				printf("%f", number);/*change to fprintf to the log file for all the things that are used in here!*/
			}

		}
		else
			printf("%c", *fmt);
        
        ++fmt;
    }
 
    va_end(args);
}/*will be put into a log file later on*/
void* process_query(void* arg){

	Thread th = *(Thread*)arg;
	while( 1 ){
		pthread_mutex_lock(&mutexQueue);
		printMsg("Thread #%d: waiting for connection\n", th.index);
		while( isQueueEmpty() ){
	        pthread_cond_wait(&condQueue, &mutexQueue);
		}
		printMsg("A connection has been delegated to thread id #%d\n", th.index);
		int fd_client_current = pop();		
		pthread_mutex_unlock(&mutexQueue);

		// printf("Server accepted the client\n");
		char buff[BUFF_SIZE];
		bzero(buff, sizeof(buff));
		int valread = 1;
		do{
			valread = read(fd_client_current, buff, sizeof(buff));
			if( valread ){
				printMsg("\nThread #%d received query %s\n", th.index, buff);
				/*now process the query*/
				char select1[] = "SELECT";
				bool reader_ = true;
				for( int i = 0; reader_ && i<strlen(select1); ++i)
					if( select1[i] != buff[i] )
						reader_ = false;

				/*now we know for sure which type of query we are talking about here*/
				if( reader_ )
					Reader(buff);
				else
					Writer(buff);
			}
			/*write the returned value now*/
		}while(valread);

		close_util(fd_client_current);

		// printf("Server accepted the client\n");
		// char buff[BUFF_SIZE];
		// bzero(buff, sizeof(buff));
		
		// do{
		// 	valread = read(client_socket_fd, buff, sizeof(buff));
		// 	if( valread )
		// 		printf("\nMessage arrived from client %s\n", buff);

		// 	write(client_socket_fd, "HELLO", 5);

		// }while(valread);


		// char to_execute = 'r';
		// printf("Thread #%d executing %c query\n", th.index, to_execute);
		// if(to_execute == 'r' || to_execute == 'R' )
		// 	Reader(to_execute);
		// else
		// 	Writer(to_execute);/*make that char 'r'*/
		sleep(0.5);
	}
}

char** Reader(const char* query){
	pthread_mutex_lock(&m);

		while((AW+WW) > 0){
			/*if there are  any writer than wait*/
			WR++;
			pthread_cond_wait(&okToRead, &m);
			WR--;
		}

		AR++;

	pthread_mutex_unlock(&m);

	printf("Process is reading from the database\n");

	/*we need this additional mutex because readers can't read exactly at the same time since the response is stored as a global variable, I would change this if I had time but i don't have it unfortunately*/
	pthread_mutex_lock(&response_db_mutex);
		QuerySQL((char*)query);
		char** answ = getResCopy();
	pthread_mutex_unlock(&response_db_mutex);
	/*do the database work that needs to be done in here!!!*/
	
	pthread_mutex_lock(&m);
		AR--;
		if( AR == 0 && WW > 0 )
			pthread_cond_signal(&okToWrite);
	pthread_mutex_unlock(&m);	

	return answ;
}

char** Writer(const char* query){
	pthread_mutex_lock(&m);
	while( (AW + AR) > 0 ){
		WW++;
		pthread_cond_wait(&okToWrite, &m);
		WW--;
	}
	AW++;
	pthread_mutex_unlock(&m);

	QuerySQL((char*)query);
	char** database_response = getResCopy();
	// printf("\n\n\nProcess is writing to the database! Task: %c\n\n\n\n", task_char);
	/*update the database now!*/
	pthread_mutex_lock(&m);
	AW--;
	if( WW > 0 )
		pthread_cond_signal(&okToWrite);
	else if( WR > 0 )
		pthread_cond_signal(&okToRead);

	pthread_mutex_unlock(&m);

	return database_response;
}