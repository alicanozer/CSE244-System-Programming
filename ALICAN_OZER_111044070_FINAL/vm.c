/*
*GIT-Department of Computer Enginnering
*Bil244 System Programming Spring-2014
*Final Project 2;
*@author Alican OZER
*@ID 111044070
*@date 24.05.2014
*@filename vm.c
*this file includes matrix verifier module
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

#define ERRRATE 0.02
#define MAINPORT 5000
#define MMMPORT 5002
#define MAXSIZE 40
#define CONSOLE stdout

typedef double** MATRIX;

/*function prototips*/
int getVerify(MATRIX first,const int size);
void setI(MATRIX matris,const int size);
void printMatrix(FILE* output,char* info,MATRIX matrix, const int size);
void MMMWorks();
MATRIX myMalloc(const int size);
void myFree(MATRIX matrix,const int size);
void allFree(const int size);
void allAlloc(const int size);
void SIGINTHandler(int signo);
void SIGPIPEHandler(int signo);

MATRIX matrixA;
MATRIX matrixInverseA;
MATRIX multiplyMatrix;
int sizeOfMatrix=0;
FILE* logFile;
int listenMAIN=0;
int mmmFD = 0;
int mainFD = 0;
int TRACEMODE = 0;
int inverted=0,nonInverted=0;
time_t tvalStart,tvalFinish;/*zaman farki icin*/

/*-----------------------------------*/ 
int main(int argc, char *argv[]){
	signal(SIGINT,SIGINTHandler);
	signal(SIGPIPE,SIGPIPEHandler);
	int n = 0,i=0,argK = 0;
	time(&tvalStart);/*for keeping starting time*/
	if (argc<1)
	{
		printf("USAGE:%s <MMMIp>\n", argv[0]);
		return 1;
	}
	char file[20];
	sprintf(file,"VM-%d.log",getpid());
	logFile=fopen(file,"w");
/*****connection definition*/
	struct sockaddr_in sockedMAIN;
	struct sockaddr_in socketMMM;
	struct hostent *clientOfMMM;	

	if((mmmFD = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		perror("Error : Socket MMM:");
		SIGINTHandler(2);
		return 1;
	}	
	clientOfMMM = gethostbyname(argv[1]);

	bzero((char *) &socketMMM, sizeof(socketMMM)); //clear the memory for server address
	socketMMM.sin_family = AF_INET;   
	bcopy((char *)clientOfMMM->h_addr,(char *)&socketMMM.sin_addr.s_addr,clientOfMMM->h_length);
	socketMMM.sin_port = htons(MMMPORT);

	if(connect(mmmFD, (struct sockaddr *)&socketMMM, sizeof(socketMMM))<0)
	{
		perror("Error : Connect MMM:");
		SIGINTHandler(2);
		return 1;
	}
	printf("Connected to Main port:%d\n",MAINPORT);


	if((listenMAIN = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		perror("Error : Socket MAIN:");
		SIGINTHandler(2);
		return 1;
	}
	bzero((char *) &sockedMAIN, sizeof(sockedMAIN)); //clear the memory for server address

	sockedMAIN.sin_family = AF_INET;
	sockedMAIN.sin_addr.s_addr = htonl(INADDR_ANY);
	sockedMAIN.sin_port = htons(MAINPORT);
	
  	bind(listenMAIN, (struct sockaddr*)&sockedMAIN,sizeof(sockedMAIN));

	if(listen(listenMAIN, 10) == -1){
		perror("Failed to listen MAIN:");
		SIGINTHandler(2);
		return -1;
	}
  
	printf("VM Module socket is ready port:%d\n",MMMPORT);
	
	
	printf("Waiting for MAİN Module connection\n");
	mainFD = accept(listenMAIN, (struct sockaddr*)NULL ,NULL);

	printf("MAİN Module Connected port:%d\n",MAINPORT);
	/*--------------TAMAM--------------*/
	printf("Running...\n");

	read(mainFD,&sizeOfMatrix,sizeof(int));
	read(mainFD,&TRACEMODE,sizeof(int));
	read(mainFD,&argK,sizeof(int));
	printf("GELEN SIZE:%d__",sizeOfMatrix );
	printf("TRACEMODE:%d\n",TRACEMODE );
	fprintf(logFile, "GELEN SIZE:%d__TRACEMODE:%d\n",sizeOfMatrix,TRACEMODE );	

	allAlloc(sizeOfMatrix);
	
	while(n<argK){
		n++;
		for (i = 0; i < sizeOfMatrix; ++i)
		{
			read(mainFD,matrixA[i],sizeOfMatrix*sizeof(double));
		}
		printMatrix(logFile,"MAINDEN GELEN A",matrixA,sizeOfMatrix);
		for (i = 0; i < sizeOfMatrix; ++i)
		{
			read(mainFD,matrixInverseA[i],sizeOfMatrix*sizeof(double));
		}
		printMatrix(logFile,"MAINDEN GELEN A-1",matrixInverseA,sizeOfMatrix);

		MMMWorks();
		fprintf(logFile,"%d->MATRiS iSLEMi BiTTi\n",n);
		if(TRACEMODE) printf("%d->MATRiS iSLEMi BiTTi\n",n);
		sleep(1);
	}
	SIGINTHandler(2);

	return 0;
}
/**************************************/
void MMMWorks(){

	int n = 0,i=0;
	
	for (i = 0; i < sizeOfMatrix; ++i)
	{
		n=write(mmmFD,matrixA[i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"MMM E GIDEN A",matrixA,sizeOfMatrix);
	for (i = 0; i < sizeOfMatrix; ++i)
	{
		n=write(mmmFD,matrixInverseA[i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"MMM E GIDEN A-1",matrixInverseA,sizeOfMatrix);

	for (i = 0; i < sizeOfMatrix; ++i)
	{
		n=read(mmmFD,multiplyMatrix[i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"MMM DEN GELEN A*A-1(~BIRIM)",multiplyMatrix,sizeOfMatrix);
	getVerify(multiplyMatrix,sizeOfMatrix);
}

void SIGINTHandler(int signo){
	printf("Exiting Properly[C^0]...\n");
	double diff_sec;
	sleep(2);
	close(listenMAIN);
	close(mmmFD);
	close(mainFD);	
	fclose(logFile);
	allFree(sizeOfMatrix);
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf("Number of inverted Matrix:%d  non-Invertable Matrix:%d\n",inverted,nonInverted );
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	exit(signo);
}

void SIGPIPEHandler(int signo){
	printf("Exiting Properly[P]...\n");
	double diff_sec;
	sleep(2);
	close(listenMAIN);
	close(mmmFD);	
	close(mainFD);
	fclose(logFile);
	allFree(sizeOfMatrix);
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf("Number of inverted Matrix:%d  non-Invertable Matrix:%d\n",inverted,nonInverted );
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	exit(signo);
}


int getVerify(MATRIX first,const int size){
	int counter,counter2;

	MATRIX unitMatrix=myMalloc(size);

	setI(unitMatrix,size);
	
	for(counter=0;counter<size;counter++){
		for(counter2=0;counter2<size;counter2++){
			if(first[counter][counter2] != unitMatrix[counter][counter2]){
				if(first[counter][counter2] - unitMatrix[counter][counter2] > ERRRATE){
					nonInverted++;
					printf(">>>-MATRiS DOGRULANMADI[%d][non-Invertable]-<<<\n",nonInverted);
					fprintf(logFile,">>>-MATRiS DOGRULANMADI[%d][non-Invertable]-<<<\n",nonInverted);
					return 0;
				}
			}
				
		}
	}
	inverted++;
	printf(">>>-MATRiS DOGRULANDI[%d][Invertable]-<<<\n",inverted);
	fprintf(logFile,">>>-MATRiS DOGRULANDI[%d][Invertable]-<<<\n",inverted);
	myFree(unitMatrix,size);
	return 1;
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
/*set given matrix to unitmatrix--*/
void setI(MATRIX matris,const int size){
	int counter,counter2;
	
	for(counter=0;counter<size;counter++){
		for(counter2=0;counter2<size;counter2++){
			if(counter==counter2)
				matris[counter][counter2]=1;
			else
				matris[counter][counter2]=0;
		}
	}
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
}
void allFree(const int size){
	myFree(matrixA,size);
	myFree(multiplyMatrix,size);
	myFree(matrixInverseA,size);
}
/*end of project2 vm part---*/