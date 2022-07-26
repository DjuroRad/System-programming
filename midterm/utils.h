#ifndef UTILS_H
#define UTILS_H

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>           /* For O_* constants */	
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


void errExit(const char* message);
int shm_open_util( const char* name, int oflag, mode_t mode );
void fstat_util( int fd, struct stat* statbuf );
void ftruncate_util( int fd, off_t length );
void* mmap_util( void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int read_util( int fd, void* buf, size_t count );
int write_util( int fd, void* buf, size_t count );
void close_util( int fd );
sem_t *sem_open_util( const char* name, int oflag, mode_t mode, unsigned int value );
int sem_init_util( sem_t* sem, int pshared, unsigned int value);
int sem_wait_util( sem_t* sem );
int sem_post_util( sem_t* sem );
int sem_close_util( sem_t* sem );
int sem_destroy_util( sem_t* sem_unnamed );
int sem_unlink_util( const char* sem_name );
int mkfifo_util(const char *pathname, mode_t mode);
int open_util(const char *pathname, int flags);
int munmap_util(void *addr, size_t length);
int unlink_util(const char *pathname);
int shm_unlink_util(const char *name);
int sigaction_util(int signum, const struct sigaction *act, struct sigaction *oldact);
#endif