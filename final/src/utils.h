#ifndef UTILS_H
#define UTILS_H

#include <sys/mman.h>
#include <sys/stat.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */	
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SA struct sockaddr_in /*used for a client and the server*/

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
int pthread_create_util(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int pthread_detach_util(pthread_t thread);
int fclose_util(FILE *stream);
int pthread_create_util(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int pthread_join_util(pthread_t thread, void **retval);
int socket_util(int domain, int type, int protocol);
int bind_util(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int accept_util(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect_util(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void *malloc_util(size_t size);
void *realloc_util(void *ptr, size_t size);
void *calloc_util(size_t nmemb, size_t size);

#endif