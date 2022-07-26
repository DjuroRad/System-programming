#include "utils.h"

void errExit(const char* message){
	fprintf(stderr, "Error message is: %s, Error number is: %d\n", message,errno);
	exit(EXIT_FAILURE);
}

int shm_open_util( const char* name, int oflag, mode_t mode ){
	int fd;
	fd = shm_open( name, oflag, mode );
	
	if( fd == -1 )
		errExit("shm_open");

	return fd;
}

void fstat_util( int fd, struct stat* statbuf ){
	if( fstat( fd, statbuf ) == -1 )
		errExit("fstat");
}

void ftruncate_util( int fd, off_t length ){
	if (ftruncate(fd, length) == -1)
		errExit("ftruncate");
}

void* mmap_util( void* addr, size_t length, int prot, int flags, int fd, off_t offset){
	void* addr_to_map;
	addr_to_map = mmap( addr, length, prot, flags, fd, offset );
	if( addr_to_map == MAP_FAILED )
		errExit("mmap");
	return addr_to_map;
}

int munmap_util(void *addr, size_t length){
	if( munmap(addr, length) == -1 )
		errExit("munmap");
	return 0;
}


int read_util( int fd, void* buf, size_t count ){
	int return_val;
	if( (return_val = read( fd, buf, count )) == -1 )
		errExit("read");

	return return_val;
}

int write_util( int fd, void* buf, size_t count ){
	int count_n;
	if( (count_n=write(fd, buf, count )) == -1 )
		errExit("write");
	return count_n;
}

void close_util( int fd ){
	if( close(fd) == -1 )
		errExit("close");
}

sem_t* sem_open_util( const char* name, int oflag, mode_t mode, unsigned int value ){
	sem_t* sem;
	if( (sem = sem_open( name, oflag, mode, value )) == SEM_FAILED )
		errExit( "sem_open" );

	return sem;
}

int sem_init_util( sem_t* sem, int pshared, unsigned int value){
	int ret;
	if( (ret = sem_init(sem, pshared, value)) == -1 )
		errExit("sem_init");
	return ret;
}

int sem_wait_util( sem_t* sem ){
	if( (sem_wait( sem )) == -1 )
		errExit("sem_wait");
	return 0;
}

int sem_post_util( sem_t* sem ){
	if( (sem_post( sem )) == -1 )
		errExit("sem_post");
	return 0;
}

int sem_close_util( sem_t* sem ){
	if( sem_close(sem) == -1 )
		errExit("sem_close");
	return 0;
}

int sem_destroy_util( sem_t* sem_unnamed ){
	if( sem_destroy( sem_unnamed ) == -1 )
		errExit("sem_destroy");
	return 0;
}

int sem_unlink_util( const char* sem_name ){
	if( sem_unlink(sem_name) == -1 )
		errExit("sem_unlink");
	return 0;
}

int mkfifo_util(const char *pathname, mode_t mode){
	if( (mkfifo( pathname, mode) == -1) && errno != EEXIST )
		errExit("mkfifo");

	return 0;
}

int open_util(const char *pathname, int flags){
	int fd;
	if( (fd = open( pathname, flags ) ) == -1 )
		errExit("open");

	return fd;
}

int unlink_util(const char *pathname){
	if( unlink( pathname ) == -1 )
		errExit("unlink");
	return 0;
}

int shm_unlink_util(const char *name){
	if( shm_unlink(name) == -1 )
		errExit("shm_unlink");
	return 0;
}