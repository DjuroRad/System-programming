#include <stdio.h>

/*using for file operations*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#define N_CHILD 8
#define N_POINTS 8

typedef enum{ ci_1, ci_2, ci_3, ci_4, ci_5, ci_6, ci_7, ci_8 } child_index;

static volatile int alive_children = 8;
int sigCnt = 0;
int children[N_CHILD];
int id=-1;	
int fd = -1;

void childProcess( int fd, int count);
void parentProcess();
void parentSignalsChildren();
void suspendSignal( int sig );
void blockSignal( int sig );
void makeHandler(  int sig, void (*handler)(int) );
void childTerminationHandler();
void insertIntoFile(int fd, const char* str_to_append, int position);
void getCoordinates( float arr[], int n, int fd, const char* delim, const char indicator, int count);
float interpolation( float* arr, int n, float xi );
int countDigits(float num);
int getPosition(int fd, int count);
void childTask(int fd, int count, int coord_n);
float getNum( int fd, int count_delim, char delim, int row );
void outputError( int fd, int degree );
void printCoefficients( float arr[], int n_points, int degree, int polynomial );
void gaussEliminationLS(int m, int n, double a[m][n], double x[]);


// static void chldHandler(int sig);
// static void parentHandler();
// static void contChildHandler(int sig);
void terminate();
static void handlerGeneral();
int main(int argc, char const *argv[])
{

	memset(children, -1, N_CHILD*sizeof(int));/*setting memory to -1 assumiing fork will fail so that it is easier to check*/
																														
	if(argc > 2)
		exErr("Too many command line arguments were provided. You should only provide one which is the file to be read and manipulated\n");

	/*Needed variable declaration*/
	int count=0;

	/*Openning a the given file using open*/
	if ( (fd = open( argv[1], O_RDWR)) < 0 ) 
		exErr("Function open didn't work for the given filename\n");


	/*
		Installing a handler for all the signals here right now!
		Handler will be called general handler
	*/
	makeHandler( SIGUSR1, &handlerGeneral );
	makeHandler( SIGINT, &terminate );
	/*
		Installing a mask to block all the signals /// *** UPDATE it will block only SIGUSR1 signal so that the children could be created
		until the children are created!
	*/
	blockSignal( SIGUSR1 );
	/*
		Now that all the signals are blocked let's make those N_CHILD children
		using fork and a for loop

		*After each child is made it will suspend!
		- Those details will be implemented in the child process 
	*/

	for(count = 0; count < N_CHILD; ++count){
		
		id = fork();

		switch(id){
			case -1:
				exErr("Error while calling the fork system call\n");
				break;
			case 0:
				childProcess(fd, count);
				break;
			default:
				children[count] = id; /*Save children' ids to be able to send them signals later on*/
				suspendSignal( SIGUSR1 );/*Making blocked signal now available so that a child process can inform the parent it finished*/

				break;
		}
	}
	/*
		Now after all the children have been created and suspended
		we have to send the SIGUSR2 signal in order to awake them
		it will be performed in a for loop
	*/
	
	if(id!=0){
		/*
			Parent should now calculate the error for each row 
			This is done for the 5th degree
		*/
		outputError( fd, 5 );
		/*
			Signal all children to continue with the second task
			Suspend parent until all children have finished
		*/
		parentSignalsChildren();
		/*
			install a handler for SIGCHLD so that we can control how many children have finished execution
		*/
		makeHandler(SIGCHLD, &childTerminationHandler);
		/*
			Suspend parent until all children have finished execution
		*/
		while( alive_children > 0 )
			suspendSignal( SIGCHLD );

		outputError( fd, 6 );

	}
	if( close(fd) == -1 )
		exErr("close() returned -1\n");
	return 0;
}

void childTerminationHandler(){
	pid_t child_pid;
	int status;
	while(  ( child_pid = waitpid(-1,&status,WNOHANG) ) > 0 )
		--alive_children;
	if( child_pid == -1 && errno != ECHILD )
		exErr("waitpid failed execution\n");
}

void childProcess( int fd, int count ){


// int n = N_POINTS*2;/*We know there are 8 children inside our file, it will be provided that way*/
// 	float arr[n];/*Now we will fill in our array*/
// 	getCoordinates( arr, n, fd, ",", '\n', 0);
	
// 	float xi = arr[n-4];/*last point is the xi we are calculating*/
// 	float res = interpolation( arr, 6, xi );/*last point is the xi we are calculating*/
// 	In order to append our result we need to convert it to a string
// 	char str_num[256];
// 	gcvt(res, countDigits(res)+1, str_num);
// 	printf("%d - STRNUM %s\n", countDigits(res), str_num );
// 	/*Add a comma before execution*/
// 	char str_to_append[256];
// 	strcpy(str_to_append, ",");

// 	strcat(str_to_append, str_num);
// 	printf("STR TO APPEND:%s\n", str_to_append);

// 	insertIntoFile( fd, str_to_append,getPosition(fd, count) );

// 	exit(1);

	makeHandler(SIGUSR2, &handlerGeneral);
	blockSignal(SIGUSR2);
	/*First thing to be done will be showing the child was created and printing its id*/
	// printf("%d - Child created and performing |   #1   - task execution\n", getpid());
	/*Performing childTask 1*/
	childTask( fd, count, 6 );


	kill(getppid(), SIGUSR1);/*Waking up the parent so that it could create a new child after which it would again be suspended*/
	
	/*
		After child is created it will be suspended until SIGUSR2 signal is received
		Now i can suspend the childprocess until SIG_USR2 is received
		SIG_SUR2 is received when all children have been made and finished with they first process
	*/
	suspendSignal(SIGUSR2);
	/*Now it will start its execution of the first process*/
	// printf("%d child |   #2   - task execution\n", count);
	childTask( fd, count, 7 );
	/*Now it will exit*/
	exit(EXIT_SUCCESS);/*This send a SIGCHLD signal to parent*/
}

void suspendSignal( int sig ){

	sigset_t sigsetmask;
	sigemptyset(&sigsetmask);
	sigdelset(&sigsetmask, sig);
	
	/*Now all the children should be put to sleep*/
	if( sigsuspend(&sigsetmask) == -1 ) /*perror("sigsuspend() returned -1 as expected");*/

	return;
}

void blockSignal( int sig ){
	sigset_t sigsetmask;
	if( sigemptyset(&sigsetmask) == -1 ) exErr("Sigfillset returned an error\n");
	if( sigaddset(&sigsetmask, sig) == -1 ) exErr("Sigfillset returned an error\n");
	if( sigprocmask(SIG_BLOCK, &sigsetmask, NULL) == -1 ) exErr("Sigprocmask returned on error\n\n\n");
}

void makeHandler(  int sig, void (*handler)(int) ){
	struct sigaction sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sa_handler = handler;
	sigaction(sig, &sa, NULL);
}


void parentSignalsChildren(){
	/*Here we can signal all the children at once and there is not need for parent to sleep now*/
	int i;
	for( i = 0; i<N_CHILD; ++i )
		kill( children[i], SIGUSR2 );

	return;
}

// static void chldHandler(int sig){
	
// }

// static void contChildHandler(int sig){
	
// }

float interpolation( float* arr, int n, float xi ){

	// printf("xi: %f\n", xi);

	double result = 0.0f;
	int i;
	for( i = 0; i<n; ++i ){
		// Compute individual terms of above formula
        double out = arr[i*2+1];

        int j;
        for ( j=0; j<n; j++ )
        {
            if ( j != i ){
                out = out*(xi - arr[j*2]) / (arr[i*2] - arr[j*2]);
                // printf("Out = %f * (%f - %f) / (%f - %f)\n", out, xi, arr[j*2],arr[i*2],arr[j*2]);
            }
            
            	// Add current term to result
        }
        result += out;
	}

	return result;
}

float avg_err_est(){

	return 0.0f;
}

static void handlerGeneral(){

}

void insertIntoFile(int fd, const char* str_to_append, int position){

	struct stat fd_stat;
	off_t fd_size;
	if( fstat(fd, &fd_stat) == -1 )
		exErr("stat() system call returned -1 indicating an error\n");
	fd_size = fd_stat.st_size;

	/*Enlargen the size of the file*/
	fd_size += strlen(str_to_append);
	if( ftruncate(fd, fd_size) == -1 )
		exErr("ftruncate() failed and returned -1\n");

	/*Now reading n bytes at a time a writing to the end of the file*/
	int len = strlen(str_to_append);
	/*
		writing( copying ) from the last byte since the file size is incremented by the length of the string that is to be added		
	*/
	int write_offset = -1;
	/*
		read offset will start from the previous end of the file which can be found here by 
		substracting the amount that was added to the file originaly
	*/
	int read_offset = write_offset - (len);/*going backwards*/	

	int cnt = 0;
	/*Moving all the elements to the right so that a string could be appended*/
	while(read_offset > ( -1 * (fd_size - position) -1 ) ){
		char ch;
		/*setting file's offset for reading*/
		if( lseek(fd, read_offset, SEEK_END) == -1 && errno != 0 ) exErr("lseek() got an error, returned -1\n");
		read(fd, &ch, sizeof(char));
		/*setting file's offset for writing*/
		if( lseek(fd, write_offset, SEEK_END) == -1 && errno != 0 ) exErr("lseek() got an error, returned -1\n");
		write(fd, &ch, sizeof(char));	

		cnt++;
		--write_offset;
		--read_offset;
	}

	// printf("CNT: %d - FSIZE: %d\n", cnt, fd_stat.st_size);
	// printf("IN HERE EVEN, strtoappend: %s, position:%d\n", str_to_append, position);

	/*Start from the end and to towards the beginning*/
	
	/*setting the file offset to the position new string should be added to*/
	if( lseek(fd, position, SEEK_SET) == -1 && errno != 0 ) exErr("lseek() returned an error\n");

	/*writing that string into the file*/
	write(fd, str_to_append, len );
}

void getCoordinates( float arr[], int n, int fd, const char* delim, const char indicator, int count){
	char buff[256];
	char ch;
	int i;
	/*Setting offset according to count, count represents which row will be taken*/
	for( i = 0; i<count; ++i){
		do{
			read(fd, &ch, sizeof(char));
		}while(ch!='\n');
	}
	/*Adding to the buffer the elements that will be placed inside our array*/
	i=0;
	do{
		read(fd, buff+i, sizeof(char));/*reading byte by byte here*/
		++i;
	}
	while(buff[i-1] != '\n');
	/*Setting the buffer's last char to be terimnation character*/
	buff[i]='\0';

	/*Now filling in the array that was passsed*/
	i = 0;
	char* token = strtok( buff, delim );
	do{
		if(token!=NULL) arr[i] = atof(token);
		++i;
		token = strtok(NULL, delim);
	}while(token != NULL);

	return;
}

int countDigits(float num){
	int count = 0;
	while(num > 1.0f){
		num /= 10;
		++count;
	}

	return count;
}
/*It will count the number of characters before counth^th new line is reached*/
int getPosition(int fd, int count){
	int i;
	int position = 0;
	for( i = -1; i<count; ++i ){
		char ch;
		do{
			++position;
			read( fd, &ch, sizeof(ch) );
		}while( ch != '\n' );
	}

	return position;
}
/*Task performed by Child | Writing the result to file depending on the number of coordinates involved here --- 6 for the first childTask and 7 for the second childTask*/
void childTask(int fd, int count, int coord_n){
	/*Lock the file since multiple children will want to access it*/
	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	fcntl(fd, F_SETLKW, &lock);
	/*Getting the elements to be calculated from the file*/
	int n = N_POINTS*2;/*We know there are 8 children inside our file, it will be provided that way*/
	float arr[n];/*Now we will fill in our array*/
	if( lseek(fd, 0, SEEK_SET) == -1 && errno != 0 ) exErr("lseek() returned -1\n");
	getCoordinates( arr, n, fd, ",", '\n', count);
	float xi = arr[n-2];/*last point is the xi we are calculating*/
	float res = interpolation( arr, coord_n, xi );/*last point is the xi we are calculating*/
	/*In order to append our result we need to convert it to a string*/
	char str_num[256];
	gcvt(res, countDigits(res)+1, str_num);
	/*Add a comma before execution*/
	char str_to_append[256];
	strcpy(str_to_append, ",");

	strcat(str_to_append, str_num);
	/*Now it can be securely added to the end of the row*/
	if( lseek(fd, 0, SEEK_SET) == -1 && errno != 0) exErr("lseek() returned -1\n");

	insertIntoFile( fd, str_to_append, getPosition( fd, count )-1);
	/*Unlocking the file now that this process finished so that the next one could use it*/
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLKW, &lock);

	/*	If it is the second round print the coefficients also	*/
	if( coord_n == 7 )
		printCoefficients(arr, N_POINTS-1, 6, count);
	return;
}

void printCoefficients( float arr[], int n_points, int degree, int polynomial ){
 // an array of size 2*n+1 for storing N, Sig xi, Sig xi^2, ...., etc. which are the independent components of the normal matrix
    //no. of data-points
    // int N;  
    //degree of polynomial
    // int n;  
    //arrays to store the c and y-axis data-points
    double x[n_points], y[n_points];  
    // printf("Enter the x-axis values:\n");
    int i,j;
    for(i=0;i<n_points;i++)
    	x[i] = arr[2*i];
   
    for(i=0;i<n_points;i++)
        y[i] = arr[2*i + 1];
   
    // an array of size 2*n+1 for storing N, Sig xi, Sig xi^2, ...., etc. which are the independent components of the normal matrix
    double X[2*degree+1];  
    for(i=0;i<=2*degree;i++){
        X[i]=0;
        for(j=0;j<n_points;j++){
            X[i]=X[i]+pow(x[j],i);
        }
    }
    //the normal augmented matrix
    double B[degree+1][degree+2];  
    // rhs
    double Y[degree+1];      
    for(i=0;i<=degree;i++){
        Y[i]=0;
        for(j=0;j<n_points;j++){
            Y[i]=Y[i]+pow(x[j],i)*y[j];
        }
    }
    for(i=0;i<=degree;i++){
        for(j=0;j<=degree;j++){
            B[i][j]=X[i+j]; 
        }
    }
    for(i=0;i<=degree;i++){
        B[i][degree+1]=Y[i];
    }
    double A[degree+1];
    printf("Polynomial %d: ",polynomial);
    // printMatrix(degree+1,degree+2,B);
    gaussEliminationLS(degree+1,degree+2,B,A);
    for(i=degree;i>=0;i--){
        printf("%.1f ",A[i]);
    }
    printf("\n");
}
/*count is for the number of delimiters*/
/*
	if count_delim is 0 last number is returned
	if count_delim > 0 than number number of commas from the \n line will be counted to the left and the number following
	count_delim^th delim ( delimeter ) will be returned
*/
float getNum( int fd, int count_delim, char delim, int row ){

	if( count_delim < 0 )
		exErr("getNum() count_delim argument has to be > 0\n");
	/*Reset the fd in case it was altered beforehand*/
	if( lseek(fd, 0, SEEK_SET) == -1 && errno != 0 ) exErr("lseek() returned -1\n");
	/*First go to the end of the line depending on the count of row we are in*/
	char ch;
	int i;
 	char buff[256];

 	/*
		Getting offset to the end of the line
 	*/
	int comma_position = 0;
	int temp = 0;/*keeping a space between two commas in this variable*/
	for( i = 0; i<row; ++i ){
		do{
			++temp;
			read(fd, &ch, sizeof(char));
			
			if( ch == ',' ){
				comma_position += temp;
				temp = 0;
			}
		}while(ch != '\n');
	}

 	if( count_delim == 0 ){

 	/*
		Will find and return the last element in the row
 	*/

		/*fd's offset is now at the end of the row where the estimated value is at its end*/
		/*Now i need to go back until the comma is reached*/
		/*I calculated where the comma i need is and now i need to set fd so that i could read that value*/
		if( lseek( fd, comma_position*sizeof(char), SEEK_SET ) == -1 && errno != 0 ) exErr("lseek() returned -1\n");
	 	/*Now go and read into the buffer until the new line*/
	 	i = 0;
	 	do{
	 		read( fd, buff + i, sizeof(char) );
	 		++i;
	 	}while( buff[i-1] != '\n' );
	 	buff[i] = '\0';
	}
 	else{

 	/*
 		Take the offset using fstat system call
 		Move it backwards afterwards until delimiters are read
 		At this moment the lseek is at the newline
 	*/

	 	while( count_delim >= 0 ){

	 		lseek(fd, -1, SEEK_CUR);
	 		read( fd, &ch, sizeof(char) );
	 		
	 		// printf("char is %c\n", ch);
	 		if( ch == ',' ){
	 			--count_delim;
	 			if( count_delim >= 0 )
	 				lseek(fd, -1, SEEK_CUR);
	 		}
	 		else
	 			lseek(fd, -1, SEEK_CUR);
	 	}

	 	i = 0;
	 	do{
	 		read( fd, buff+i, sizeof(char) );
	 		// printf("BUFF2[i] %c\n", buff[i]);
	 		++i;
	 	}while( buff[i-1] != ',' );
	 	buff[i-1] = '\0';
	 	// printf("BUFF2 %s\n", buff);
 	}
 	/*
		Return the file descriptor to its start
 	*/

 	return atof(buff);
}

void outputError( int fd, int degree ){
	int i;
	float res = 0.0f;
	for( i = 0; i<N_CHILD; ++i ){
		float y_estimation = getNum( fd, 0, ',', i+1 );
		float y;
		if( degree == 5 )
			y = getNum( fd, 1, ',', i+1 );
		else
			y = getNum( fd, 2, ',', i+1 );

		res += fabs(y_estimation - y);

	}
	res /= N_CHILD;
	printf("Error of polynomial of degree %d: %.1f\n", degree, res);
	return;
}

/*
	 Function that performs Gauss-Elimination and returns the Upper triangular matrix and solution of equations:
	There are two options to do this in C.
	*Pass the augmented matrix (a) as the parameter, and calculate and store the upperTriangular(Gauss-Eliminated Matrix) in it.
*/
void gaussEliminationLS(int m, int n, double a[m][n], double x[n-1]){
    int i,j,k;
    for(i=0;i<m-1;i++){
        //Partial Pivoting
        for(k=i+1;k<m;k++){
            //If diagonal element(absolute vallue) is smaller than any of the terms below it
            if(fabs(a[i][i])<fabs(a[k][i])){
                //Swap the rows
                for(j=0;j<n;j++){                
                    double temp;
                    temp=a[i][j];
                    a[i][j]=a[k][j];
                    a[k][j]=temp;
                }
            }
        }
        //Begin Gauss Elimination
        for(k=i+1;k<m;k++){
            double  term=a[k][i]/ a[i][i];
            for(j=0;j<n;j++){
                a[k][j]=a[k][j]-term*a[i][j];
            }
        }
         
    }
    //Begin Back-substitution
    for(i=m-1;i>=0;i--){
        x[i]=a[i][n-1];
        for(j=i+1;j<n-1;j++){
            x[i]=x[i]-a[i][j]*x[j];
        }
        x[i]=x[i]/a[i][i];
    }
             
}

void terminate(){
	if( id == 0 )
		kill(getppid(), SIGINT);
	/*In parent terminate all processes*/
	int i;
	for(i = 0; i<N_CHILD; ++i){
		if( children[i] != -1 )
			kill( children[i], SIGKILL );
	}
	if( fd != -1 )
		close(fd);
	
	exit(EXIT_FAILURE);
}