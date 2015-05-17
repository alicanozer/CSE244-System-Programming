/*
*GIT-Department of Computer Enginnering
*Bil244 System Programming Spring-2014
*Final Project 2;
*@author Alican OZER
*@ID 111044070
*@date 24.05.2014
*@filename mmm.c
*this file includes matrix multiplication module
*/
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <semaphore.h>

// default values
#define CONSOLE stdout
#define MAINPORT 5001
#define VMPORT 5002
#define MAXSIZE 40

typedef double** MATRIX;

/*FUNCTION PROTOTYPES*/
void multipMatrix(MATRIX result,MATRIX matrix1,MATRIX matrix2,const int size);
void printMatrix(FILE* output,char* info,MATRIX matrix, const int size);
void MAINModuleWorker();
void VMModuleWorker();
MATRIX myMalloc(const int size);
void myFree(MATRIX matrix,const int size);
void allFree(const int size);
void allAlloc(const int size);

void SIGINTHandler(int signo);
void SIGPIPEHandler(int signo);
void *threadFunction(void* args);

/*GLOBAL VARIABLES*/
int sizeOfMatrix=0;
sem_t sema;
MATRIX matrixA;
MATRIX multiplyMatrix;
MATRIX matrixInverseA;
MATRIX matrixInverseL;
MATRIX matrixInverseU;
int listenVM = 0,vmFD = 0;
int listenMAIN = 0,mainFD = 0;
int TRACEMODE = 0,count = 0;
FILE* logFile;
pthread_t *myThreads;
time_t tvalStart,tvalFinish;/*zaman farki icin*/

int main(int argc, char *argv[]){

	signal(SIGINT,SIGINTHandler);
	signal(SIGPIPE,SIGPIPEHandler);
	int argP=0,argK = 0,treadCounter = 0;
	/*-------connection definition--------------*/ 
	struct sockaddr_in socketMAIN;
	struct sockaddr_in socketVM;
	time(&tvalStart);/*for keeping starting time*/
	char log[20];
	sprintf(log,"MMM-%d.log",getpid());
	logFile=fopen(log,"w");

	listenMAIN = socket(AF_INET, SOCK_STREAM, 0);
	listenVM = socket(AF_INET, SOCK_STREAM, 0);

	bzero((char *) &socketVM, sizeof(socketVM)); //clear the memory for server address
	bzero((char *) &socketMAIN, sizeof(socketMAIN)); //clear the memory for server address

	socketMAIN.sin_family = AF_INET;    
	socketMAIN.sin_addr.s_addr = htonl(INADDR_ANY); 
	socketMAIN.sin_port = htons(MAINPORT);

	socketVM.sin_family = AF_INET;    
	socketVM.sin_addr.s_addr = htonl(INADDR_ANY); 
	socketVM.sin_port = htons(VMPORT);

	bind(listenMAIN, (struct sockaddr*)&socketMAIN,sizeof(socketMAIN));
	bind(listenVM, (struct sockaddr*)&socketVM,sizeof(socketVM));

	if(listen(listenMAIN, 10) == -1){
		perror("Failed to listen MAIN:");
		SIGINTHandler(2);
		return -1;
	}

	printf("MAIN Module socket is ready port:%d\n",MAINPORT);

	if(listen(listenVM, 10) == -1){
		perror("Failed to listen VM:");
		SIGINTHandler(2);
		return -1;
	}
	printf("VM Molude socket is ready port:%d\n",VMPORT);	      
	
	printf("Waiting for VM Module connection\n");
	vmFD = accept(listenVM, (struct sockaddr*)NULL ,NULL);
	printf("VM Module connected.\n");

	// accept awaiting request
	printf("Waiting for MAIN Module connection\n");
	mainFD = accept(listenMAIN, (struct sockaddr*)NULL ,NULL);
	printf("MAIN Module connected.\n");

	/*-------initialize semaphore-------*/
    if(sem_init(&sema, 0, 0) == -1){
    	perror("Failed to initialize semaphore!!!");
    	SIGINTHandler(2);
    	return 1;
    }
    printf("Running...\n");

	/*-----------reading some args from main process------*/
	read(mainFD,&sizeOfMatrix,sizeof(int));
	read(mainFD,&argK,sizeof(int));
	read(mainFD,&argP,sizeof(int));
	read(mainFD,&TRACEMODE,sizeof(int));

	printf("GELEN SIZE:%d__",sizeOfMatrix );
	printf("ThreadNumber:%d__",argP );
	printf("TRACEMODE:%d\n",TRACEMODE );
	fprintf(logFile, "GELEN SIZE:%d__ThreadNumber:%d__TRACEMODE:%d\n",sizeOfMatrix,argP,TRACEMODE );

	allAlloc(sizeOfMatrix);
	
	myThreads=(pthread_t*)malloc(argP*sizeof(pthread_t));

	/*create and detach several threads*/
	for (treadCounter = 0; treadCounter < argP; ++treadCounter)
	{
		pthread_create(myThreads+treadCounter,NULL,threadFunction,NULL);
		pthread_detach(myThreads[treadCounter]);
	}	

	count = 0;
	while(count<argK){
		sem_post(&sema);
		usleep(10000);
		sem_wait(&sema);		
	}
	SIGINTHandler(2);
	
	return 0;
}
/*------------------------*/
void *threadFunction(void* args){
	while(1){
		sem_wait(&sema);
			count++;
			MAINModuleWorker();
			VMModuleWorker();
		sem_post(&sema);
		usleep(10000);
	}

}
void MAINModuleWorker(){
	int i=0;
	for (i = 0; i < sizeOfMatrix; ++i)
		{
			read(mainFD,matrixInverseU[i],sizeOfMatrix*sizeof(double));
		}
		printMatrix(logFile,"MAİNDEN GELEN U-1",matrixInverseU,sizeOfMatrix);
		for (i = 0; i < sizeOfMatrix; ++i)
		{
			read(mainFD,matrixInverseL[i],sizeOfMatrix*sizeof(double));
		}
	
		printMatrix(logFile,"MAİNDEN GELEN L-1",matrixInverseL,sizeOfMatrix);
		/*multply comes matrix and sent it to MAIN Module*/
		multipMatrix(multiplyMatrix,matrixInverseU,matrixInverseL,sizeOfMatrix);
		printMatrix(logFile,"U-1*L-1",multiplyMatrix,sizeOfMatrix);

		for (i = 0; i < sizeOfMatrix; ++i)
		{
			write(mainFD,multiplyMatrix[i],sizeOfMatrix*sizeof(double));
		}			

		printMatrix(logFile,"MAINE GIDEN U-1*L-1",multiplyMatrix,sizeOfMatrix);
}

void VMModuleWorker(){
	int i=0,n=0;			
				
	for (i = 0; i < sizeOfMatrix; ++i)
	{
		read(vmFD,matrixA[i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"VM DEN GELEN A",matrixA,sizeOfMatrix);
	for (i = 0; i < sizeOfMatrix; ++i)
	{
		read(vmFD,matrixInverseA[i],sizeOfMatrix*sizeof(double));
	}

	printMatrix(logFile,"VM DEN GELEN A-1",matrixInverseA,sizeOfMatrix);

/*multply comes matrix and sent it to VM Module*/
	multipMatrix(multiplyMatrix,matrixA,matrixInverseA,sizeOfMatrix);

	for (i = 0; i < sizeOfMatrix; ++i)
	{
		write(vmFD,multiplyMatrix[i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"VM YE GIDEN A*A-1(BIRIM)",multiplyMatrix,sizeOfMatrix); 
	fprintf(logFile,"%d->MATRiS iSLEMi BiTTi\n",count);
	if(TRACEMODE) printf("%d->MATRiS iSLEMi BiTTi\n",count);
	sleep(1);
	
}
void SIGPIPEHandler(int signo){
	printf("Exiting Properly[P]...\n");
	double diff_sec;
	sleep(2);
	close(listenVM);
	close(vmFD);
	close(mainFD);
	close(listenMAIN);
	allFree(sizeOfMatrix);
	free(myThreads);
	fclose(logFile);
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	exit(signo);
}
void SIGINTHandler(int signo){
	printf("Exiting Properly[C^0]...\n");
	double diff_sec;
	sleep(2);
	close(listenVM);
	close(vmFD);
	close(mainFD);	
	close(listenMAIN);
	allFree(sizeOfMatrix);
	free(myThreads);
	fclose(logFile);
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	exit(signo);
}
void multipMatrix(MATRIX result,MATRIX matrix1,MATRIX matrix2,const int size){
	int counter,counter2,k;
	
	for(counter=0;counter<size;counter++)
	{
	    for(counter2=0;counter2<size;counter2++)
	    {
	        result[counter][counter2]=0;
	        for(k=0;k<size;k++)
	        {
	            result[counter][counter2]+=matrix1[counter][k] * matrix2[k][counter2];
	        }
	    }
	}
}
// print matrix
void printMatrix(FILE* output,char* info,MATRIX matrix,const int size) {
	int row, column;

	fprintf(output, "\nBiLGi:%s\n",info );
	for(row = 0; row < size; row++) {
		for(column = 0; column < size; column++) {
			fprintf(output,"%6.2f   ", matrix[row][column]);
		}
		fprintf(output,"\n");
	}
	fprintf(output,"\n");
}
MATRIX myMalloc(const int size){
	int i=0;
	double** matrix;
	matrix=(double**)malloc(size*sizeof(double*));
	if (matrix == NULL){
		perror("Error:allocating memory:");
		SIGINTHandler(2);
	}

	for (i = 0; i < size; ++i)
	{
		matrix[i]=(double*)malloc(size*sizeof(double));
		if (matrix[i] == NULL)
		{
			perror("Error:allocating memory:");
			SIGINTHandler(2);
		}
	}
	return matrix;
}
void myFree(MATRIX matrix,const int size){
	int i=0;
	for (i = 0; i < size; ++i)
		free(matrix[i]);
	free(matrix);
}
void allAlloc(const int size){
	matrixA=myMalloc(size);
	multiplyMatrix=myMalloc(size);
	matrixInverseA=myMalloc(size);
	matrixInverseL=myMalloc(size);
	matrixInverseU=myMalloc(size);
}
void allFree(const int size){
	myFree(matrixA,size);
	myFree(multiplyMatrix,size);
	myFree(matrixInverseA,size);
	myFree(matrixInverseL,size);
	myFree(matrixInverseU,size);
}