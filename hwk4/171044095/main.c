/*
	evey actor is is a separate thread
*/


/* 
	Thread H - read homeworks one by one from an input file
	Put all homeworks in a priority queue ( speed S, cost C, quality Q )
*/


/*
	Arbitrary numbers of students for hire ***** Each a separate thread *****

	Student-for-hire: | price_per_homework | speed_per_homework | quality_per_homework |
*/


/*
	Main thread - depending on priority gives task to student-for-hire thread ( most suitable and available one / NOT BUSY WITH ANOTHER HOMEWORK )
*/





/*
	Two files:
	1. 
			contains homework and priority
			at least one homework, arbitrary number!

	2.
			names and properities of the students-for-hire
*/
#define BUFF_SIZE 256
#define QUALITY 'Q'
#define SPEED 'S'
#define COST 'C'

#define queue_sem "QUEUE_ACCESS"
#define students_for_hire_sem "BUSY_STUDENTS"

#include "types.h"
#include "parsearg.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include <sys/types.h>
#include <sys/wait.h>

typedef struct Student{
	char name[BUFF_SIZE];
	int cost;
	int quality;
	int speed;
	int index;
	int homework_done_count;
	bool busy;
	char current_homework;
	int current_wallet;
	
	sem_t sem;
	sem_t available;
}Student;

void init_student( Student* student, int speed, int cost, int quality );
void init( int argc, char** argv );
void init_students( Student students_for_hire[] );
int getStudentsNum( int fd_ );
void* cheating_student( void* arg );
void* student_hired( void* arg );
void init_semaphores();
void main_thread();
void clean();
void handler();

int getBestChoice( char hwk_to_do );
int fastestStudent();
int bestQualityStudent();
int cheapestStudent();
int getNonBusyStudent();

sem_t queue_access;
sem_t students_sem;

sem_t sem_empty;
sem_t sem_full;

int wallet = 0;
int min_cost, min_cost_i;
int students_n = 0;
Student* students_for_hire=NULL;
bool finished = false;
int fd = -1;
FILE* fp = NULL;
int fd_student = -1;
int fd_homeworks = -1;

int main( int argc, char** argv ){

	struct sigaction sact;
	memset(&sact, 0, sizeof(sact));
	sact.sa_handler = handler;

	sigaction_util(SIGINT, &sact, NULL);
	

	init( argc, argv );
	students_for_hire = (Student*)malloc(students_n*sizeof(Student));
	if( students_for_hire == NULL )
		errExit("malloc : students-for-hire");

	init_students( students_for_hire );/*read students from file!*/
	init_semaphores();
	
	pthread_attr_t attribute;
	pthread_attr_init(&attribute);
	pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);

	pthread_t cheating_student_thread;
	pthread_t students_thread[students_n];
	/*main thread starts here*/

	/*creating cheating student thread*/
	pthread_create_util( &cheating_student_thread, &attribute, &cheating_student, NULL );


	min_cost = students_for_hire[0].cost;

	for( int i = 0; i<students_n; ++i ){
		pthread_create_util( &students_thread[i], &attribute, &student_hired, &students_for_hire[i] );
		if( min_cost > students_for_hire[i].cost ){
			min_cost = students_for_hire[i].cost;
			min_cost_i = i;
		}

		students_for_hire[i].index = i;
	}

	// printf("MIN COST IS %d \n\n", min_cost);
	/*consumer*/
	main_thread();
	
	for(int i =0; i<students_n; ++i )
		pthread_join(students_thread[i], NULL);
	
	sem_post_util( &sem_empty );/*make sure it is unlocked if waitin*/
	pthread_join( cheating_student_thread, NULL);

	printf("\nHomeworks solved and money made by the students\n");

	int count = 0;
	int total = 0;
	for( int i = 0; i < students_n; ++i ){
		printf("%s %d %d\n", students_for_hire[i].name, students_for_hire[i].homework_done_count, students_for_hire[i].cost * students_for_hire[i].homework_done_count );
		count += students_for_hire[i].homework_done_count;
		total += students_for_hire[i].cost * students_for_hire[i].homework_done_count;
	}
	printf("Total cost for %d homeworks %dTL\n", count, total);
	printf("Money left at G's account: %dTL\n", wallet);
	clean();

	return EXIT_SUCCESS;
}

void main_thread(){

	while( !finished ){


		sem_wait_util( &sem_full );

		sem_wait_util( &queue_access );
			if( isQueueEmpty() )
				break;
			char hwk_to_do = pop();
		sem_post_util( &queue_access );
		/*process this homework*/
		/*find student to process it*/
		int best_choice;

		sem_wait_util(&students_sem);
			best_choice = getBestChoice( hwk_to_do );
		if( wallet < students_for_hire[best_choice].cost ){/*when money ain't sufficiant wait for the cheapest one*/
			best_choice = min_cost_i;
			sem_wait_util( &students_for_hire[min_cost_i].available );
			sem_post_util( &students_for_hire[min_cost_i].available );/*post it afterwards immediately so that there is no deadlock*/
		}
		/*will be popped in the thread!!!*/

		/*change the state of the student*/
		students_for_hire[best_choice].busy = true;
		students_for_hire[best_choice].current_homework = hwk_to_do;

		wallet = wallet - students_for_hire[best_choice].cost;
		students_for_hire[best_choice].homework_done_count += 1;
		students_for_hire[best_choice].current_wallet = wallet;
		sem_post_util(&students_for_hire[best_choice].sem);
		/*give a sign that */
		/*pay the student*/
		/*remove this homework from the queue*/

		if( wallet < min_cost ){
			finished = true;
			printf("H has no more money for homeworks, terminating.\n");
			break;
		}

		/*decrese the amout of money*/
		sem_post_util( &sem_empty );
		
	}

	for(int i = 0; i<students_n; ++i)
		sem_post_util( &students_for_hire[i].sem );

}
/*producer in this case*/
void* cheating_student( void* arg ){

	fd_homeworks = open_util( args[hwk_flg], O_RDONLY);
	int read_n = 1;

	char ch;

	while( !finished ){

		sem_wait_util( &sem_empty );
		
		// printf("IN HERE");
			if( !finished && ( read_n = read_util(fd_homeworks, &ch, sizeof(char) ) ) != 0 && (ch =='C' || ch == 'S' || ch =='Q' ) ) {

				sem_wait_util(&queue_access);
					push(ch);
					// printf("ADDED TO QUEUE %d %d\n", ch, read_n);
				sem_post_util(&queue_access);

			}
			else
				finished = true;

		sem_post_util( &sem_full );

	}

	close_util(fd_homeworks);
	fd_homeworks = -1;

	if( read_n == 0 || !(ch =='C' || ch == 'S' || ch =='Q' ))
		printf("H has no other homeworks, terminating\n");

	return NULL;
}

void* student_hired( void* arg ){

	Student* hired_student = (Student*) arg;

	while( !finished ){

		printf("\n%s is waiting for homework\n", hired_student->name);
		sem_wait_util(&hired_student->sem);/*indicator from the main process*/
		if(finished)
			break;
		sem_wait_util(&hired_student->available);/*go to sleep to indicate it ain't available*/
			// hired_student -> busy = true;
			
			printf("%s is solving homework %c for %d, H has %dTL\n", hired_student->name, hired_student->current_homework,hired_student->cost, hired_student->current_wallet);

			int sleep_time = 6 - (hired_student -> speed);
			sleep( sleep_time );

		
		hired_student -> busy = false;
		
		sem_post_util(&hired_student->available);/*indicate that it finished*/
		sem_post_util(&students_sem);
	}

	return NULL;
}

int getBestChoice( char hwk_to_do ){

	int best_choice;

	switch(hwk_to_do){
		case SPEED:
			best_choice = fastestStudent();
			break;
		case QUALITY:
			best_choice = bestQualityStudent();
			break;
		case COST:
			best_choice = cheapestStudent();
			break;
		}

	return best_choice;
}

void init_students( Student students_for_hire[] ){
	fd = open_util( args[student_flg], O_RDONLY );
	fp = fdopen( fd, "r" );

	char* buffer = NULL;
	char line[BUFF_SIZE];

	int i = 0;
	while( fgets(line, sizeof(char)*BUFF_SIZE, fp ) != NULL ){
		
		int j = 0;

		buffer = strtok( line, " " );
		while( buffer != NULL ){
			switch( j ){
				case 0:
					strcpy( students_for_hire[i].name, buffer );
					break;
				case 1:
					students_for_hire[i].quality = atoi( buffer );
					break;
				case 2:
					students_for_hire[i].speed = atoi( buffer );
					break;
				case 3:
					students_for_hire[i].cost = atoi( buffer );
					break;
			}
			buffer = strtok( NULL, " " );
			++j;
		}
		
		++i;
	}
	close_util(fd);
	fd = -1;
	fclose(fp);
	fp = NULL;

	printf("%d students-for-hire threads have been created.\n", students_n);
	printf("Name\t Q\t S\t C\n");
	for( int i = 0; i<students_n; ++i )
		printf("%s %d %d %d\n", students_for_hire[i].name, students_for_hire[i].quality, students_for_hire[i].speed, students_for_hire[i].cost );
	
	return;
}

void init( int argc, char** argv ){
	parseArgs(argc, argv);
	wallet = atoi( args[cash_flg] );	

	fd_student = open_util( args[student_flg], O_RDONLY);

	students_n = getStudentsNum(fd_student);

	return;
}

int getStudentsNum( int fd_ ){
	int count = 0;
	char ch;

	while( read_util( fd_, &ch, sizeof(char) ) ){
		if( ch == '\n' )
			++count;
	}
	
	return count;
}
void init_student( Student* student, int speed, int cost, int quality ){
	student -> speed = speed;
	student -> quality = quality;
	student -> cost = cost;
	student -> homework_done_count = 0;
	student -> busy = false;
}
void handler(){
	printf("Termination signal received, closing.\n");
	clean();
}
void clean(){
	free_queue();
	
	for( int i = 0; i<students_n; ++i ){
		sem_destroy_util(&students_for_hire[i].sem);
		sem_destroy_util(&students_for_hire[i].available);
	}

	if( students_for_hire != NULL )
		free(students_for_hire);
	
	students_for_hire = NULL;

	sem_destroy_util( &sem_empty );
	sem_destroy_util( &sem_full );
	sem_destroy_util( &queue_access );
	sem_destroy_util( &students_sem );
	

	if( fd_student != -1 )
		close_util(fd_student);

	if (fd_homeworks != -1)
		close_util(fd_homeworks);

	if( fd != -1 )
		close_util(fd);

	if( fp != NULL )
		fclose(fp);

	fp = NULL;
	fd_student = -1;
	fd_homeworks = -1;
	fd = -1;
}

int fastestStudent(int limit){
	
	int choice = getNonBusyStudent();

	for( int i = 0; i<students_n; ++ i ){
		if( students_for_hire[i].speed > students_for_hire[choice].speed && !students_for_hire[i].busy )
			choice = i;	
	}

	return choice;
}
int bestQualityStudent(int limit){
	
	int choice = getNonBusyStudent();
	
	for( int i = 0; i<students_n; ++ i ){
		if( students_for_hire[i].quality > students_for_hire[choice].quality && !students_for_hire[i].busy )
			choice = i;	
	}

	return choice;
}
int cheapestStudent(int limit){
	int choice = getNonBusyStudent();
	
	for( int i = 0; i<students_n; ++ i ){
		if( students_for_hire[i].cost < students_for_hire[choice].cost && !students_for_hire[i].busy )
			choice = i;	
	}

	return choice;
}

/*returns -1 if there is no free students!*/
int getNonBusyStudent(){
	for( int i = 0; i<students_n; ++i )
		if( !students_for_hire[i].busy )
			return i;

	return -1;
}
void init_semaphores(){

	sem_init_util( &students_sem, 0, students_n );
	sem_init_util( &sem_full, 0, 0);
	sem_init_util( &sem_empty, 0, 1);
	sem_init_util( &queue_access, 0, 1 );

	for( int i = 0; i<students_n; ++i ){
		sem_init_util( &students_for_hire[i].sem, 0, 0 );
		sem_init_util( &students_for_hire[i].available, 0, 1 );
	}
}


