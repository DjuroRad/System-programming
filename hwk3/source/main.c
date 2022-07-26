#include "parsearg.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define BUFF_SIZE 50
#define FIFOPERM (S_IRUSR | S_IWUSR)
#define SHARED_MEM_SIZE 2056
#define UNNAMED_ "unnamedsemaphore212121571"

typedef struct process_info{
	pid_t pid;
	char fifoname[BUFF_SIZE];
	int switch_n;
}process_info;

/*Function declarations*/
int choose_fifo( char fifonames[][BUFF_SIZE] );/*it will return the index of the chosen fifo*/
unsigned int get_n_fifos();
void set_proc_info( char fifoname[BUFF_SIZE] );
int choose_random_fifo( char fifonames[][BUFF_SIZE] );
bool areAllPotatosCold( void* shared_mem_seg, int shared_fd );
void decrease_switch( pid_t recived_pid, void* shared_mem_seg, int shared_fd );
void loadFifoNames( char fifonames[][BUFF_SIZE], int n );
short getSharedCount( void* shared_addr );
short incrementSharedCount( void* shared_addr );
short decrementSharedCount( void* shared_addr );
process_info getProcInfoFromFifoName( void* shared_addr, char fifoname[BUFF_SIZE] );
process_info getProcInfoFromPID( void* shared_addr, pid_t current_potato_id);
void cleanup();/*to be called before the program exits!*/
int getNameIndex( int i_rand_fifo, int count );

/*Global variables*/
unsigned int n_fifonames;
process_info proc_info;
sem_t* sem = NULL;
sem_t* sem_unnamed = NULL;
int start_offset;
int fifo_fd_read = -1;
int fd = -1;
void* addr = NULL;
char fifonamefirst[BUFF_SIZE];

int main( int argc, char* argv[] ){

	struct sigaction handler;
	handler.sa_handler = cleanup;

	sigaction( SIGINT, &handler, NULL);
	/*used for choosing a random fifo*/
	srand(time(NULL));
	/*setup atexit functionality*/
	if( atexit(cleanup) != 0 )
		errExit("atexit");

	parseArgs( argc, argv );
	// printf("B: %s, S: %s, F: %s, M: %s\n", args[bflg], args[sflg], args[fflg], args[mflg] );
	n_fifonames = get_n_fifos();
	int fifo_fd_write[n_fifonames];/*multiple write fifos and one read fifo*/

	char fifonames[n_fifonames][BUFF_SIZE];
	loadFifoNames( fifonames, n_fifonames );/*works fine*/
	strcpy( fifonamefirst, fifonames[0] );

	int fifo_fd_read;
	start_offset = sizeof(short) + sizeof(sem_t);/*at first two positions i will need an unnamed semaphore and the shared counter for synchronization*/
	sem = sem_open( args[mflg], O_CREAT, 0666, 1 );/*opent the named semaphore we got*/
	sem_unnamed = sem_open( UNNAMED_, O_CREAT, 0666, 0 );
	// sem_unnamed = sem_open( UNNAMED, O_CREAT, 0666, 0 );
	int fd, flags = O_CREAT | O_RDWR;
	mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
	
	fd = shm_open_util( args[sflg], flags, perms);/*Open shared memory first*/
	/*
		bflg	-	has potato or not
		sflg	-	shared memory name
		fflg	-	file with fifo names
		mflg	-	name of the semaphore
	*/

	/*
		If semaphore allows writting
		process will write its id that
		will be associated with that potato
		and the number of switches needed

		this info is written in SHARED mem
	*/

	/*
		Each process will have its own fifo
		Use shared memory in order to 
		give a different fifo to each process
		
	*/



	
	

	/*
		Every process will write its potato id and potato number in shared memory in the following format:
		
		pid switch_n\n
	*/
	
	
	
	/*
		Link shared memory afterwards
		In order to link the shared memory
		I need size of the shared memory
		Size is the previous size of the shared memory
		Plus new size to be added.
	*/
	
	/*First time when shared memory is used its size will be 0 and we will write the shared variable into it, NOTE: This will be only done for the first program that is executing*/
	sem_wait_util(sem);
	
	struct stat statbuf;
	fstat_util( fd, &statbuf );
	/*checek if it is the first time opening the shared memory segment and truncate the size of this file if it is*/
	if( statbuf.st_size == 0 ){
		// printf("SHARED MEMORY IS 0");
		short shared_var = 0;/*it will be increase with each process*/
		ftruncate_util( fd, SHARED_MEM_SIZE );
		addr = mmap_util( NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
		memcpy( addr, &shared_var, sizeof(short) );/*writting shared variable at 0 ( first ) position*/
		memcpy( addr + sizeof(short), &sem_unnamed, sizeof(sem_t));
		// munmap_util(addr, start_offset);/*unmap it now since we will change the size again*/
	}
	else{
		// printf("SHARED MEMORY NOT 0");
		addr = mmap_util( NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	}
	// printf("OUT OF THIS\n");

	sem_post_util(sem);




	// printf("AFTER POST");
	// sem_wait_util(sem);/*Now wait to get permissions to write the fifo information inside shared memory*/
	// printf("AFTER WAIT");
	
	short count;
	/* Find a fifo you can connect for reading */
	// printf("\nBEFORE CHOOSE FIFO\n");
	count = choose_fifo(fifonames);/*before this shared memory segment is truncated in order to fit the data needed*/
	/*count updated inside choose fifo function*/
	// printf("\nAFTER CHOOSE FIFO\n");
	// printf("AFTER CHOOSE FIFO\n");
	/*After this we can open our fifo*/
	




	mkfifo_util( proc_info.fifoname, 0666 );/*make it with reading and writing ends*/
	// fifo_fd_read = open_util( proc_info.fifoname, O_RDONLY | O_NONBLOCK );/*setting it with nonblock so that we can write immediately to it*/
	


	sem_post_util( sem );

	// printf("COUNT IS %d   |   N_FIFONAMES IS %d\n", count, n_fifonames);
	/*after this wait until all the fifos are created, when the last process is executed it can post the semaphore*/
	if( count == n_fifonames - 1){
		int ind;
		for( ind = 0; ind<count; ++ind ){
			// printf("POST UNNAMED SEMAPHORE COUNT %d\n", count);
			// int fd_connect_end;
			// fd_connect_end = open_util( fifonames[ind], O_WRONLY );/*setting it with nonblock so that we can write immediately to it*/
			/*this way fifos will be connected*/
			sem_post_util( sem_unnamed );
		}
	}
	else{
		// printf("NOW INFINITE WAITING");
		sem_wait_util( sem_unnamed );
	}

	// printf("\n\n\n%d CONNECTING ALL FILES\n\n\n", getpid());
	int fifo_i;
	int j=0;
	for( fifo_i = 0; fifo_i < n_fifonames; ++fifo_i ){
		if( fifo_i == count )
			fifo_fd_read = open_util( fifonames[fifo_i], O_RDONLY );/*setting it with nonblock so that we can write immediately to it*/
		else{/*will get here    n_fifonames-1    times*/
			fifo_fd_write[j] = open_util( fifonames[fifo_i], O_WRONLY );/*setting it with nonblock so that we can write immediately to it*/
			++j;
		}
	}
	// printf("\n\n\n%d: NOW ALL THE FIlES ARE CONNECTED, COUNT: %d\n\n\n",getpid(), count);
	// printf("AFTER EVERYTHING");
	
	

	pid_t current_potato_id = proc_info.pid;
	pid_t recived_potato_id;
	process_info info_potato = proc_info;/*for the first time writing*/

	bool cooled_down = false;
	while( !cooled_down ){
		/*storing the value of this potato i got*/
	
		int i_rand_fifo = choose_random_fifo(fifonames);/*this is for finding random one to WRITE to and there is N - 1 of them so i need to find its index in the original fifonames array*/
		int i_fifoname = getNameIndex( i_rand_fifo, count );/*finding the appropriate index*/

		// sem_wait_util( sem );		
		// 	process_info random_info = getProcInfoFromFifoName( addr, fifonames[i_fifoname] );
		// sem_post_util( sem );

		
		if( info_potato.switch_n > 0 ){/*we will send the potato only if it is didn't cool down*/
			printf("\npid = %d sending potato number %d to %s; this is switch number: %d \n", proc_info.pid, info_potato.pid, fifonames[i_fifoname], info_potato.switch_n );
			write_util( fifo_fd_write[ i_rand_fifo ] , &current_potato_id, sizeof(pid_t) );/*send potato to another process*/
		}
		/*Recieve a potato*/
		sem_wait_util( sem );	
			cooled_down = areAllPotatosCold(addr, fd);
			if(cooled_down) break;/*check before receiving something in order to avoid deadlock*/
			
			read_util( fifo_fd_read, &recived_potato_id, sizeof(pid_t) );/*now recieve the potato from another process*/
		sem_post_util( sem );
		// printf("\n\n RECIEVED POTATO %d\n\n", recived_potato_id);
		current_potato_id = recived_potato_id;/*recived potato id*/
		sem_wait_util( sem );			
			info_potato = getProcInfoFromPID( addr, current_potato_id);
			if( info_potato.switch_n > 0 )
			printf("\npid = %d receiving potato number %d; this is switch number %d\n", proc_info.pid, current_potato_id, info_potato.switch_n);

		sem_post_util( sem );

		/*num of switches is decreased in the memory*/
		info_potato.switch_n = info_potato.switch_n -1;
		if( info_potato.switch_n == 0 )
			printf("pid=%d; potato number %d has cooled down\n", proc_info.pid, recived_potato_id);

		sem_wait_util( sem );
			decrease_switch( recived_potato_id, addr, fd );
			cooled_down = areAllPotatosCold(addr, fd);
		sem_post_util( sem );
		
	}
	
	
	// printf("\n%d Unlinking IN AT EXIT\n",getpid());
	printf("Closing all file descriptors!\n");
	for( int i = 0; i<n_fifonames-1; ++i)
		close_util(fifo_fd_write[i]);
	close_util(fifo_fd_read);
	close_util(fd);
	munmap_util(addr, SHARED_MEM_SIZE);
	return 0;
}



/*
	parameter shared_mem_seg is using the shared memory segment in order to check 
	for the already used fifos
*/
int choose_fifo( char fifonames[][BUFF_SIZE] ){
	
	short count;/*shared count to be extracted from shared memory segment!*/
	memcpy( &count, addr, sizeof(short) );
	set_proc_info( fifonames[count] );
	
	// printf("%d (count) got fifo: %s\n", count, proc_info.fifoname);
	// printf("increment count and save it and new process's info into shared memory segment");
	memcpy( addr + start_offset + count*sizeof(process_info), &proc_info, sizeof(process_info) );
	count++;
	memcpy( addr, &count, sizeof(short) );		
	
	return count-1;
}

unsigned int get_n_fifos(){

	int fd = open( args[fflg], O_RDONLY);

	unsigned int line = 0;
	int eof = -1;
	char dummy;
	while( eof != 0 ){

		eof = read_util( fd, &dummy, sizeof(char));
		if(dummy == '\n' && eof != 0)
			++line;
	}
		
	return line+1;
}

void set_proc_info( char fifoname[BUFF_SIZE] ){
	proc_info.pid = getpid();
	// printf("TEMP IS %s\n", temp);
	strcpy( proc_info.fifoname, fifoname );
	proc_info.switch_n = atoi( args[bflg] );
}

int choose_random_fifo( char fifonames[][BUFF_SIZE] ){

	/*choose from 0 to n_fifonames - 1 since that is the number of descriptors for writing*/
	int rand_num = (rand()+getpid())%(n_fifonames-1);
	
	// if( strcmp(fifonames[rand_num], proc_info.fifoname) == 0 )
	// 	rand_num = (rand_num + 1) % (n_fifonames-1);
	
	return rand_num;
}

bool areAllPotatosCold( void* shared_mem_seg, int shared_fd ){
	
	struct stat statbuf;
	fstat_util( shared_fd, &statbuf );
	bool cooled_down = true;

	int i;
	// for( i = 0; cooled_down && (i*sizeof( process_info ) + start_offset)< size; ++i ){
	for( i = 0; cooled_down && i<n_fifonames; ++i ){
		// printf(" IN LOOP ");
		process_info process_shared;
		memcpy( &process_shared, shared_mem_seg + i*sizeof(process_info) + start_offset, sizeof(process_info) );

		if( process_shared.switch_n > 0 )
			cooled_down = false;
	}

	return cooled_down;
}

void decrease_switch( pid_t recived_pid, void* shared_mem_seg, int shared_fd ){
	struct stat statbuf;
	fstat_util( shared_fd, &statbuf );
	bool updated = false;
	int i;
	/*first we need to find the pid and than we have to write decrease value to that segment*/
	// for( i = 0; !updated && (i*sizeof( process_info )+ start_offset) < size; ++i ){
	for( i = 0; !updated && i<n_fifonames; ++i ){
		// printf(" IN LOOP ");
		process_info process_shared;
		memcpy( &process_shared, shared_mem_seg + i*sizeof(process_info) + start_offset, sizeof(process_info) );

		if( process_shared.pid == recived_pid ){
			updated = true;
			process_shared.switch_n = process_shared.switch_n - 1;
			memcpy( shared_mem_seg + i*sizeof(process_info)+ start_offset, &process_shared, sizeof(process_info) );
		}
	}

	return;
}

void loadFifoNames( char fifonames[][BUFF_SIZE], int n ){

	int fd = open( args[fflg], O_RDONLY);

	int eof = -1;

	int i;
	for( i = 0; i<n_fifonames; ++i ){
		int j = 0;
		char dummy = '-';
		while( eof != 0 && dummy != '\n' ){

			eof = read_util( fd, &dummy, sizeof(char));

			if(dummy == '\n' || eof == 0)
				fifonames[i][j] = '\0';
			else
				fifonames[i][j]=dummy;
			++j;
		}
	}		

	return;
}

short getSharedCount( void* shared_addr ){
	short count;

	memcpy( &count, shared_addr, sizeof(short) );

	return count; 
}

short incrementSharedCount( void* shared_addr ){
	short count;
	memcpy( &count, shared_addr, sizeof(short) );
	count++;
	memcpy(shared_addr, &count, sizeof(short) );

	return count;
}

short decrementSharedCount( void* shared_addr ){
	short count;
	memcpy( &count, shared_addr, sizeof(short) );
	count--;
	memcpy(shared_addr, &count, sizeof(short) );

	return count;
}

process_info getProcInfoFromFifoName( void* shared_addr, char fifoname[BUFF_SIZE] ){
	process_info info;
	int i;
	bool found = false;
	for( i = 0 ; !found && i<n_fifonames; ++ i){
		memcpy( &info, shared_addr + start_offset + i*sizeof(process_info), sizeof( process_info) );
		if( strcmp( info.fifoname, fifoname) )
			found = true;
	}

	return info;
}

process_info getProcInfoFromPID( void* shared_addr, pid_t current_potato_id){
	process_info info;
	int i;
	bool found = false;
	for( i = 0 ; !found && i<n_fifonames; ++ i){
		memcpy( &info, shared_addr + start_offset + i*sizeof(process_info), sizeof( process_info) );
		if( info.pid == current_potato_id )
			found = true;
	}

	return info;
}

void cleanup(){
	printf("Exiting\n");
	unlink_util(proc_info.fifoname);/*unlinking fifo made*/
	
		

	if( strcmp( proc_info.fifoname, fifonamefirst ) == 0 ){/*only one process will have this property making sure we don't unlink shared memory more than once*/
		shm_unlink_util(args[sflg]);
		sem_unlink_util(args[mflg]);
		sem_unlink_util(UNNAMED_);
	}
}

int getNameIndex( int i_rand_fifo, int count ){

	if( i_rand_fifo >= count )
		return i_rand_fifo + 1;
	else
		return i_rand_fifo;

}





