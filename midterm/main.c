#include <string.h>
#include <sys/wait.h>

#include "parsearg.h"
#include "utils.h"

#define VACCINE_KIND 2
#define VACCINE_1 '1'
#define VACCINE_2 '2'

enum sem_named{
	buffer_access_semaphore,
	buffer_empty,/*vaccinator and nurse producer/consumer ipc*/
	buffer_full,/*vaccinator and nurse producer/consumer ipc*/
	citizen_info,
	citizen_sem,
	vaccinator_sem,
	start_sem
};


struct citizen_schedule{
	int t;
	pid_t pid;
	int vac_count;
	bool needs_vac1;
	bool needs_vac2;
};
#define SEM_NUM 7
const char semaphore_names[SEM_NUM][32] = {"BUFFER_ACCESS","BUFFER_EMPTY","BUFFER_FULL", "CITIZEN_INFO", "CITIZEN", "VACCINATOR", "START"};
sem_t* semaphores[SEM_NUM]; 

const char BUFFER_NAME[] = "vaccine_storage"; /*this will be our shared memory segment for all the vaccines nurses bring*/
const char SEMAPHORE_MEM[] = "unnamed_semaphores";
const char CITIZEN_SCHEDULE[] = "citizen_vaccinator_ipc";
const char SHARED_PID_MEM_NAME[] = "shared_pid_memory";

int BUFFER_SIZE;/*it will be assigned at the begining of the program*/
int CITIZEN_SCHEDULE_SIZE;
int fd_storage;
void* storage;
void* v_c_ipc;
void* shared_pid_memory;

static volatile int numLiveChildren = 0;/*number of children start but not yet waited on*/
/*main processes that will need to be syncronized*/
void init();/*creates shared resources*/
void vaccinator( int i );
void nurse( int i );
void citizen( int i );
void* createSharedBuffer(const char* NAME, int size);
void putVaccine( char new_vaccine, int* count1, int* count2 );
void removeVac( int i );
void giveVaccine( char vac );
bool hasVaccine( char vac );
char getCitizen( int i, int vac_i );
void init_citizen( pid_t pid, int i );
void vaccinateCitizen( int i, char vac );

pid_t nurses[2000];
int n_i =0;
pid_t vaccinators[2000];
int v_i =0;

static void sigchldHandler(int sig){
	pid_t childPid;
	int status;
	while( (childPid = waitpid(-1, &status, WNOHANG)) > 0 ){
		--numLiveChildren;
		// printf("%d |||exited|||\n", childPid);
		for( int i = 0; i<atoi(args[nflg]); ++i){
			if( childPid == nurses[i] ){
				for( int j=0; j<atoi(args[nflg]); ++j)
					kill( nurses[j], SIGKILL );
			}
			if( childPid == vaccinators[i] ){
				for( int j=0; j<atoi(args[vflg]); ++j)
					kill( vaccinators[j], SIGKILL );
			}
		}
	}
	if( childPid == -1 && errno != ECHILD )
		errExit("waitpid");
}

void letKidsCreate(){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigchldHandler;
	if( sigaction( SIGCHLD, &sa, NULL ) == -1 )
		errExit("sigaction");


	/*block so that if process exits before the parent calls sigsuspend*/
	sigset_t blockMask;
	sigemptyset( &blockMask );
	sigaddset( &blockMask, SIGCHLD );

	if( sigprocmask(SIG_SETMASK, &blockMask, NULL ) == -1 )
		errExit("sigprocmask");
}

void waitKids(){
	int cnt = 0;
	sigset_t emptyMask;
	sigemptyset( &emptyMask );
	while( numLiveChildren > 0 ){
		if( sigsuspend( &emptyMask ) == -1 && errno != EINTR )
			errExit("sigsuspend");
		cnt++;
	}
	// printf("%d Children executed and terminated", cnt);

}

void clean_destroy(){
	munmap_util( storage, BUFFER_SIZE );
	munmap_util( v_c_ipc, CITIZEN_SCHEDULE_SIZE );
	
	shm_unlink_util( BUFFER_NAME );
	for( int i = 0; i<SEM_NUM; ++i ){
		sem_close_util( semaphores[i] );
		sem_unlink( semaphore_names[i] );
	}
	return;
}
void handler(int sig){
	printf("\n[ ALL ] CTRL + C (%d). Exiting...", sig);
	clean_destroy();
	exit(EXIT_FAILURE);
}

int main( int argc, char* argv[] ){
	// clean_destroy();
	// exit(0);
	/*setup our signal handler*/
	struct sigaction act;
	memset (&act, '\0', sizeof(act));
	act.sa_handler = handler;
	sigaction_util( SIGINT, &act, NULL);
	parseArgs( argc, argv );
	numLiveChildren = atoi(args[cflg]) + atoi(args[vflg]) + atoi(args[nflg]);

	init();
	// char curr_vac;
	// memcpy( &curr_vac, storage, sizeof(char) );
	// printf("%c", curr_vac);
	// clean_destroy();
	// exit(0);

	letKidsCreate();

	for( int i = 0; i<atoi(args[cflg]); ++i ){/*citizens*/
		int pid;
		if( (pid = fork()) == 0){
			// printf("CITIZEN %d\n", getpid());
			citizen( i + 1 );
			exit(EXIT_SUCCESS);
		}
		else
			init_citizen( pid, i );/*this way we make sure that all children are made on time*/
	}

	for( int i = 0; i < atoi(args[nflg]) ; ++i ){
		pid_t pid;
		if( (pid = fork()) == 0 ){
			// printf("NURSE %d\n", getpid());
			nurse( i + 1 );
			if( i == 0 )
				printf("All nurses have finished their job\n");

			exit(EXIT_SUCCESS);
		}
		else{
			nurses[i] = pid;
		}
	}

	for( int i = 0; i < atoi(args[vflg]) ; ++i ){
		pid_t pid;
		if( (pid = fork()) == 0 ){
			// printf("VACCINATOR %d\n", getpid());
			vaccinator( i + 1 );
			// sem_wait_util( semaphores[buffer_full] );
			sem_post_util( semaphores[buffer_empty] );
			exit(EXIT_SUCCESS);
		}
		else
			vaccinators[i] = pid;
	}

	waitKids();
	clean_destroy();
	return EXIT_SUCCESS;
}

void nurse( int i ){/*Nurse is a producer in our paradigm and it brings the vaccines to the vaccinator*/
	char vaccine_storage;
	int return_value=-1;
	/*first wait till the buffer is empty */
	while( return_value != 0 ){
		sem_wait_util( semaphores[buffer_empty] );
		sem_wait_util( semaphores[buffer_access_semaphore] );
		return_value = read_util( fd_storage, &vaccine_storage, sizeof(char) );
		
		if(return_value != 0){
			int vac1_count, vac2_count;
			putVaccine( vaccine_storage, &vac1_count, &vac2_count );/*add a new vaccine to the buffer, at the last place*/
			if(vaccine_storage == '1' || vaccine_storage == '2' )
				printf("Nurse %d (pid=%d) has brought vaccine %c: the clinic has %d vaccine1 and %d vaccine2.\n", i, getpid(), vaccine_storage, vac1_count, vac2_count);
		}
		sem_post_util( semaphores[buffer_access_semaphore] );
		sem_post_util( semaphores[buffer_full] );
	}

}

void vaccinator( int i ){
	/*let a new citizen in etc.*/

	/*call the citizen only when there are at least 2 vaccines in the buffer*/

	/*check if needed vaccine is in the buffer
		Block acces to the buffer while doing this
	*/
	int citizen_no = 1;
	int citizen_i;
	int cnt = 0;
	char vac;

	bool finished = true;
	do{/*this will be done until all the citizens are vaccinated*/

		if(!finished)
			sem_post_util(semaphores[vaccinator_sem]);

		sem_wait_util(semaphores[citizen_sem]);

			memcpy(&citizen_i, shared_pid_memory, sizeof(int));

			citizen_i--;
			vac = getCitizen(citizen_i, i);
		
		sem_wait_util( semaphores[buffer_access_semaphore] );
		
		bool has_vaccine = hasVaccine( vac );
		if( citizen_i < 0 ){
				
				has_vaccine = true;
			}
		if( has_vaccine ){
			giveVaccine( vac );
			vaccinateCitizen( citizen_i, vac );/*this would update everything including the number of citizens*/
			
			cnt++;
			sem_post_util( semaphores[buffer_access_semaphore] );

		}
		else{
			/*let nurses access it now*/
			sem_post_util( semaphores[buffer_access_semaphore] );
			/*check if the buffer is full first ( if nurse has brought the vaccine )*/
			do{
				sem_wait_util( semaphores[buffer_full] );
				sem_wait_util( semaphores[buffer_access_semaphore] );
				has_vaccine = hasVaccine( vac );
				if( has_vaccine ){ 
					
					giveVaccine( vac );
					vaccinateCitizen( citizen_i, vac );/*this would update everything including the number of citizens*/
					++cnt;
				}
				sem_post_util( semaphores[buffer_access_semaphore] );
				sem_post_util( semaphores[buffer_empty] );
			}while( !has_vaccine );
		}
			memcpy( &citizen_no, v_c_ipc + CITIZEN_SCHEDULE_SIZE - sizeof(int), sizeof(int) );/*check number of citizens left for vaccination*/
			if(citizen_no <= 0)
				finished = true;
			else
				finished = false;
			
		sem_post_util( semaphores[start_sem] );
	}while( !finished );
	printf("Vaccinator %d (pid=%d) vaccinated %d doses.\n", i, getpid(), cnt);

	/*when all children are out leave a sign to the vaccinators that need to also get out*/
	// int quitsignal = -1;
	// memcpy( shared_pid_memory, &quitsignal, sizeof(int));
	sem_post_util(semaphores[citizen_sem]);/*free the locked vaccinator*/

	exit(EXIT_SUCCESS);
}
void citizen( int i ){

	/*write data to shared memory segment first!!!!*/
	/*Use a semaphore for it!!!!*/
	struct citizen_schedule citizen_schedule1;

	do{
		sem_wait_util( semaphores[citizen_info] );/*have an access to buffer so check how many vaccines are left there for you*/
		sem_wait_util( semaphores[vaccinator_sem] );/*wait for the vaccinator to vaccinate you, to call you in*/
			memcpy( shared_pid_memory, &i, sizeof(int) );
		sem_post_util(semaphores[citizen_sem]);
		sem_wait_util( semaphores[start_sem] );
			memcpy( &citizen_schedule1, v_c_ipc + (i-1)*sizeof(struct citizen_schedule), sizeof( struct citizen_schedule ) );/*check how many times you need to be vaccinated again*/
		sem_post_util( semaphores[citizen_info] );/*have an access to buffer so check how many vaccines are left there for you*/
		
		
	}while(citizen_schedule1.t > 0);
	
	exit(EXIT_SUCCESS);
}

char getCitizen( int i, int vac_i ){
	/*before calling this method its access was blocked ( to the buffer I mean )*/
	struct citizen_schedule citizen_sch;
	memcpy( &citizen_sch, v_c_ipc + i*sizeof(struct citizen_schedule), sizeof(struct citizen_schedule));
	printf("Vaccinator %d (pid=%d) is inviting citizen pid=%d to the clinic", vac_i, getpid(), citizen_sch.pid);
	if( citizen_sch.needs_vac1 )
		return '1';
	else
		return '2';
}
bool hasVaccine( char vac ){

	char curr_vac='-';
	for( int i = 0; curr_vac != '\0' && i<BUFFER_SIZE; ++i ){
		memcpy( &curr_vac, storage+i, sizeof(char) );
		if( curr_vac == vac ){
			memcpy( &curr_vac, storage+1, sizeof(char) );
			return true;
		}
	}
	return false;
}
void giveVaccine( char vac ){
	char curr_vac='-';
	bool found = false;
	for( int i = 0; !found && curr_vac!='\0' && i<BUFFER_SIZE; ++i ){
		memcpy( &curr_vac, storage+i, sizeof(char) );
		if( curr_vac == vac ){
			removeVac( i );
			found = true;
		}
	}

	return;
}
void removeVac( int i ){
	for( int start = i; start < BUFFER_SIZE-1; start++ )/*don't care for zeros being copied to zeros, works and that's fine. I would have to call memcpy 2 times otherwise*/
		memcpy( storage + start, storage + start + sizeof(char), sizeof(char) );
}
void putVaccine( char new_vaccine, int* count1, int* count2 ){

	*count1 = 0;
	*count2 = 0;
	char curr_vac='-';
	int i;
	for( i = 0; curr_vac != '\0' && i < BUFFER_SIZE; ++i ){
		memcpy( &curr_vac, storage + i, sizeof(char) );
		if( curr_vac == '1' )
			*count1 = *count1 + 1;
		else if( curr_vac == '2' )
			*count2 = *count2 + 1;
	}
	--i;
	/*after finding a free place put a new vaccine in its place*/
	memcpy( storage+i*sizeof(char), &new_vaccine, sizeof(char) );
	if( new_vaccine == '1' )
		*count1 = *count1 + 1;
	else
		*count2 = *count2 + 1;

	return;
}

void init(){/*creates shared resources*/
	/*for nurse - vaccinator commnucatation*/
	BUFFER_SIZE = atoi(args[bflg]);
	/*for citien - vaccinator communication*/
	int c = atoi(args[cflg]);
	CITIZEN_SCHEDULE_SIZE = c*sizeof( struct citizen_schedule ) + sizeof(int);/*last byte will be for the number of citizens left*/

	v_c_ipc = createSharedBuffer(CITIZEN_SCHEDULE, CITIZEN_SCHEDULE_SIZE);/*will be here now*/
	shared_pid_memory = createSharedBuffer( SHARED_PID_MEM_NAME, sizeof(pid_t));/*will be here now*/

	
	memcpy(v_c_ipc + CITIZEN_SCHEDULE_SIZE - sizeof(int), &c, sizeof(int));
	int i = 0;
	memcpy(shared_pid_memory, &i, sizeof(int));

	storage = createSharedBuffer(BUFFER_NAME, BUFFER_SIZE);/*crate buffer(shared memory segment)*/

	fd_storage = open_util( args[iflg], O_RDONLY );
	/*now open semaphores that will be used for IPC*/
	for( int i = 0; i<SEM_NUM; ++i){
		if( i == buffer_full || i == citizen_sem || i == start_sem )
			semaphores[i] = sem_open_util( semaphore_names[i], O_CREAT, 0644, 0);
		else
			semaphores[i] = sem_open_util( semaphore_names[i], O_CREAT, 0644, 1);
	}

	return;
}
void* createSharedBuffer( const char* NAME, int size ){

	int flags = O_RDWR | O_CREAT;
	mode_t permissions = 0666;
	int fd = shm_open_util( NAME, flags, permissions );
	ftruncate_util( fd, size );/*create the shared mmemory segment and assign it its size*/
	void* addr = mmap_util( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	close_util( fd );/*close the unneeded file descriptor of course*/

	return addr;
}

void init_citizen( pid_t pid, int i ){
	struct citizen_schedule cit_schedule;
	cit_schedule.pid = pid;
	cit_schedule.t = atoi(args[tflg]);
	cit_schedule.needs_vac1 = true;
	cit_schedule.needs_vac2 = true;
	cit_schedule.vac_count = 0;

	
	memcpy( v_c_ipc + i*sizeof(struct citizen_schedule), &cit_schedule, sizeof( struct citizen_schedule ) );
}

void getVacCount( int* v1, int* v2 ){
	char dummy;
	*v1 = 0;
	*v2 = 0;
	for( int i = 0; i*sizeof(char)<BUFFER_SIZE; ++i ){
		memcpy( &dummy, storage + i*sizeof(char), sizeof(char));
		if( dummy == '1' )
			*v1 = *v1 + 1;
		else if( dummy == '2' )
			*v2 = *v2 + 1;
	}
}
void vaccinateCitizen( int i, char vac ){
	
	struct citizen_schedule citizen_sch;
	memcpy( &citizen_sch, v_c_ipc + i*sizeof(struct citizen_schedule), sizeof( struct citizen_schedule ) );
	if( vac == '1' )
		citizen_sch.needs_vac1 = false;
	else if( vac == '2' ){
		if( citizen_sch.t > 1 )
			citizen_sch.needs_vac1 = true;
		else
			citizen_sch.needs_vac2 = false;
		citizen_sch.t = citizen_sch.t - 1;
	}

	citizen_sch.vac_count = citizen_sch.vac_count + 1;

	int v_1, v_2;
	getVacCount(&v_1, &v_2);
	printf("\nCitizen %d (pid=%d) is vaccinated for the %d. time: the clinic has %d vaccine1 and %d vaccine2\n", i+1, citizen_sch.pid, citizen_sch.vac_count, v_1, v_2);

	if( citizen_sch.t <= 0 ){/*one citizen less!*/
		int count;
		memcpy(&count, v_c_ipc + CITIZEN_SCHEDULE_SIZE - sizeof(int), sizeof(int) );
		count--;
		memcpy(v_c_ipc + CITIZEN_SCHEDULE_SIZE - sizeof(int), &count, sizeof(int) );
		printf("The citizen is leaving. Remainin citizens to vaccinate: %d", count);
		
	}

	printf("\n");

	memcpy(v_c_ipc + i*sizeof(struct citizen_schedule), &citizen_sch, sizeof(struct citizen_schedule) );

	
	return;
}