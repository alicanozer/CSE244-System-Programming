/*
*GIT-Department of Computer Enginnering
*Bil244 System Programming Spring-2014
*Final Project 2;
*@author Alican OZER
*@ID 111044070
*@date 24.05.2014
*@filename main.c
*this file includes random matrix producer module
*LU decomposition module and
*Inverse taking module
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
#define MINIMUM 10
#define MAXIMUM 60
#define CONSOLE stdout
#define MMMPORT 5001
#define VMPORT 5000
#define MAXSIZE 40

typedef double** MATRIX;
typedef double*** QUEUE;

// functions
void setI(MATRIX matris, const int size);
void createMatrix(MATRIX matrix, const int size);
void copyLine(double copy[MAXSIZE],double original[MAXSIZE], const int size);
void printMatrix(FILE* output,char* info,MATRIX matrix, const int size);
void copyMatrix(MATRIX copy,MATRIX original, const int size);
void inverseMatrix(MATRIX inverse,MATRIX original, const int size);
void swapRows(MATRIX matrix,const int row1, const int row2, const int size);
void luDecomposition(MATRIX matrix,MATRIX lower,MATRIX upper, const int size);
void VMMoludeProducer();
MATRIX myMalloc(const int size);
void myFree(MATRIX matrix,const int size);
void allFree(const int size);
void allAlloc(const int size);
void SIGINTHandler(int signo);
void SIGPIPEHandler(int signo);

/*thread functions*/
void* fillList(void* args);
void *RMPModule(void* args);
void *LUDCModule(void* args);
void *ITModule(void* args);

/*GLOBAL VARIABLES FOR RMP-LUDM-ITM MODULE*/
QUEUE listA;
QUEUE listL;
QUEUE listU;
QUEUE listInverseA;
MATRIX matrixInverseL;
MATRIX matrixInverseU;
int sizeOfMatrix = 0,argL = 0;
int mmmFD = 0, vmFD = 0, TRACEMODE = 0;
int matrixCounter=0,target = 0;
FILE* logFile;
time_t tvalStart,tvalFinish;/*zaman farki icin*/
pthread_mutex_t safe = PTHREAD_MUTEX_INITIALIZER;

/*-******************************************/
int main(int argc, char const *argv[]){

	signal(SIGINT,SIGINTHandler);
	signal(SIGPIPE,SIGPIPEHandler);
	struct sockaddr_in socketMMM;
	struct hostent *clientOfMMM;
	struct sockaddr_in socketVM;
	struct hostent *clientOfVM;	
	pthread_t filler;
	char file[20];
	int i = 0,argP = 0,argK = 0;
	int waited=0,counter = 0;
	srand(time(NULL));// generate different numbers
    time(&tvalStart);/*for keeping starting time*/

/*---------argument controller--------------*/
	if (argc != 8)
	{
		printf("usage:%s <MMM-IP> <VM-IP> <N> <K> <L> <P> <T>\n",argv[0] );
		return -1;
	}
	sizeOfMatrix = atoi(argv[3]);
	if (sizeOfMatrix > MAXSIZE)
	{
		printf("!!!!VERILEN MATRIS BOYUTU 40 TAN BUYUK OLMAMALI.!!!\n");
		return -1;
	}
	argK = atoi(argv[4]);
	argL = atoi(argv[5]);
	argP = atoi(argv[6]);
	TRACEMODE = atoi(argv[7]);	
	sprintf(file,"MAIN-%d.log",getpid());
	logFile=fopen(file,"w");
	/**********************************/
	allAlloc(sizeOfMatrix);	
	pthread_create(&filler,NULL,fillList,NULL);
	usleep(500000);//tredin bitmesinin bekler

	pthread_t threadID[3];/*storing createed threads id*/
	
	/*-------connection definition--------------*/	
	if((mmmFD = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		perror("Error : Socket MMM:");
		SIGINTHandler(2);
		return -1;
	}
	printf("MMM Module socket is ready port:%d\n",MMMPORT);
	clientOfMMM = gethostbyname(argv[1]);

	bzero((char *) &socketMMM, sizeof(socketMMM)); //clear the memory for server address
	socketMMM.sin_family = AF_INET;
	bcopy((char *)clientOfMMM->h_addr,(char *)&socketMMM.sin_addr.s_addr,clientOfMMM->h_length);
	socketMMM.sin_port = htons(MMMPORT);

	if((vmFD = socket(AF_INET, SOCK_STREAM, 0))< 0)
	{
		perror("Error : Socket VM:");
		SIGINTHandler(2);
		return -1;
	}
	clientOfVM = gethostbyname(argv[2]);
	bzero((char *) &socketVM, sizeof(socketVM)); //clear the memory for server address
	socketVM.sin_family = AF_INET;
	bcopy((char *)clientOfVM->h_addr,(char *)&socketVM.sin_addr.s_addr,clientOfVM->h_length);
	socketVM.sin_port = htons(VMPORT);
	if(connect(mmmFD, (struct sockaddr *)&socketMMM, sizeof(socketMMM))<0)
	{
		perror("Error : Connect MMM:");
		SIGINTHandler(2);
		return -1;
	}
	printf("Connected to MMM port:%d\n",MMMPORT);
	if(connect(vmFD, (struct sockaddr *)&socketVM, sizeof(socketVM))<0)
	{
		perror("Error : Connect VM:");
		SIGINTHandler(2);
		return -1;
	}
	printf("Connected to VM module:%d\n",VMPORT );
	printf("Running...\n");
/*****************reading some args to other process***********/
	write(mmmFD,&sizeOfMatrix,sizeof(int));//send matrix size
	write(mmmFD,&argK,sizeof(int));//send matrix number
	write(mmmFD,&argP,sizeof(int));//sent tread number
	write(mmmFD,&TRACEMODE,sizeof(int));//send trace info

	write(vmFD,&sizeOfMatrix,sizeof(int));//send matrix size
	write(vmFD,&TRACEMODE,sizeof(int));//send trace info
	write(vmFD,&argK,sizeof(int));//send matrix number
			
	matrixCounter = 0;
	target = 0;
	while(matrixCounter<argK){	
		matrixCounter++;	
		
		for (i = 0; i < sizeOfMatrix; ++i)
		{
			write(mmmFD,matrixInverseU[i],sizeOfMatrix*sizeof(double));
		}
		printMatrix(logFile,"MMM-YE GIDEN U-1",matrixInverseU,sizeOfMatrix);
		for (i = 0; i < sizeOfMatrix; ++i)
		{
			write(mmmFD,matrixInverseL[i],sizeOfMatrix*sizeof(double));
		}

		printMatrix(logFile,"MMM-YE GIDEN L-1",matrixInverseL,sizeOfMatrix);

		for (i = 0; i < sizeOfMatrix; ++i)
		{
			read(mmmFD,listInverseA[target][i],sizeOfMatrix*sizeof(double));
		}

		printMatrix(logFile,"MMM-DEN GELEN U-1*L-1",listInverseA[target],sizeOfMatrix);		

		VMMoludeProducer();
		fprintf(logFile,"%d->MATRiS iSLEMi BiTTi\n",matrixCounter);
		if(TRACEMODE) printf("%d->MATRiS iSLEMi BiTTi\n",matrixCounter);

		target++;
		if(target >= argL)
			target=0;

		if (argK<=argL) waited=argK-matrixCounter;
		else{
			if (matrixCounter > (argK-argL))
				waited = argK - matrixCounter;
			else waited=argL;
		}

		if(TRACEMODE) printf("KUYRUKLARDAKi MATRiS SAYISI:%d\n",waited);	
		counter = 0;
		
		pthread_create(&threadID[counter++], NULL, RMPModule, NULL); 
		usleep(10000);

	    pthread_create(&threadID[counter++], NULL, LUDCModule, NULL);
	    usleep(10000);

	    pthread_create(&threadID[counter++], NULL, ITModule, NULL);
	    sleep(1);	
	}
	printf("TOPLAM %d TANE MATRIS UZERINDE ISLEM YAPILDI.\n",argK);
	SIGINTHandler(2);

	return 0;
}
/**********************************/
void VMMoludeProducer(){

	int i=0;

	printMatrix(logFile,"VM YE GIDEN A",listA[target],sizeOfMatrix);	

	for (i = 0; i < sizeOfMatrix; ++i)
	{
		write(vmFD,listA[target][i],sizeOfMatrix*sizeof(double));
	}
	printMatrix(logFile,"VM YE GIDEN A-1",listInverseA[target],sizeOfMatrix);
	for (i = 0; i < sizeOfMatrix; ++i)
	{
		write(vmFD,listInverseA[target][i],sizeOfMatrix*sizeof(double));
	}	
}
/**************************************/
void *RMPModule(void* args){

	pthread_mutex_lock(&safe);
		createMatrix(listA[target],sizeOfMatrix);
	pthread_mutex_unlock(&safe);
}
void *LUDCModule(void* args){

	pthread_mutex_lock(&safe);
		luDecomposition(listA[target],listL[target],listU[target],sizeOfMatrix);
	pthread_mutex_unlock(&safe);
}
void *ITModule(void* args){

	pthread_mutex_lock(&safe);
		inverseMatrix(matrixInverseL,listL[target],sizeOfMatrix);
		inverseMatrix(matrixInverseU,listU[target],sizeOfMatrix);
	pthread_mutex_unlock(&safe);
}
/*********************************/
/*firstly fill the created matrix array---*/
void* fillList(void* args){
	int i=0;
	/* L kadar matrise yer actik*/
	listA=(QUEUE)malloc(argL*sizeof(MATRIX));
	listL=(QUEUE)malloc(argL*sizeof(MATRIX));
	listU=(QUEUE)malloc(argL*sizeof(MATRIX));
	listInverseA=(QUEUE)malloc(argL*sizeof(MATRIX));	
	for (i = 0; i < argL; ++i)
	{
		listA[i]=myMalloc(sizeOfMatrix);
		listL[i]=myMalloc(sizeOfMatrix);
		listU[i]=myMalloc(sizeOfMatrix);
		listInverseA[i]=myMalloc(sizeOfMatrix);
		createMatrix(listA[i],sizeOfMatrix);
		luDecomposition(listA[i],listL[i],listU[i],sizeOfMatrix);
	}
	inverseMatrix(matrixInverseL,listL[0],sizeOfMatrix);
	inverseMatrix(matrixInverseU,listU[0],sizeOfMatrix);
	pthread_exit(NULL);
}

/*----alocate memory for a matrix---*/
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
/*--free ing a matrix size--*/
void myFree(MATRIX matrix,const int size){
	int i=0;
	for (i = 0; i < size; ++i)
		free(matrix[i]);
	free(matrix);
}
/*--alooc for l ad u matrices*/
void allAlloc(const int size){
	matrixInverseL=myMalloc(size);
	matrixInverseU=myMalloc(size);
}
/*free all reversed memory---*/
void allFree(const int size){

	int i=0;
	for (i = 0; i < argL; ++i){
		myFree(listA[i],size);
		myFree(listL[i],size);
		myFree(listU[i],size);
		myFree(listInverseA[i],size);
	}
	free(listA);
	free(listL);
	free(listU);
	free(listInverseA);
	myFree(matrixInverseL,size);
	myFree(matrixInverseU,size);
}
void SIGINTHandler(int signo){
	double diff_sec;
	printf("Exiting Properly[C^0]...\n");
	close(vmFD);
	close(mmmFD);
	allFree(sizeOfMatrix);
	fclose(logFile);
	pthread_mutex_destroy(&safe);	
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	
	exit(signo);
}
void SIGPIPEHandler(int signo){
	printf("Exiting Properly[P]...\n");
	double diff_sec;
	close(vmFD);
	close(mmmFD);
	allFree(sizeOfMatrix);
	fclose(logFile);
	pthread_mutex_destroy(&safe);
	time(&tvalFinish);/*for time different */
	diff_sec = difftime (tvalFinish,tvalStart);
	printf ("Total Elepsed time %.2lf seconds.\n", diff_sec );
	exit(signo);
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
// create matrix
void createMatrix(MATRIX matrix,const int size) {
	int row, column;

	for(row = 0; row < size; row++) {
		for(column = 0; column < size; column++) {
			matrix[row][column] = (rand() % (MAXIMUM-MINIMUM+1)) + MINIMUM;
		}
	}
}
/*-inverse given original martix to inverse matrix*/
void inverseMatrix(MATRIX inverse,MATRIX original,const int size){
	double d,k;
	int counter,counter2,x;
	MATRIX copy=myMalloc(size);
	
	setI(inverse,size);
	copyMatrix(copy,original,size);
	
	for(counter=0;counter<size;counter++){
		d=copy[counter][counter];
		for(counter2=0;counter2<size;counter2++){
			copy[counter][counter2]=copy[counter][counter2]/d;
			inverse[counter][counter2]=inverse[counter][counter2]/d;
		}
		for(x=0;x<size;x++){
			if(x != counter){
				k=copy[x][counter];
				for(counter2=0;counter2<size;counter2++){
					copy[x][counter2]=copy[x][counter2]-(copy[counter][counter2]*k);
					inverse[x][counter2]=inverse[x][counter2]-(inverse[counter][counter2]*k);
				}
			}
		}
	}
	myFree(copy,size);
}
/*--set given matris to unitmatrix*/
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
/*-copy given original matrix to matrix copy---*/
void copyMatrix(MATRIX copy,MATRIX original,const int size){
	int counter,counter2;
	
	for(counter=0;counter<size;counter++){
		for(counter2=0;counter2<size;counter2++){
			copy[counter][counter2]=original[counter][counter2];
		}
	}
}
/*-calculate l and u of given matrix--*/
void luDecomposition(MATRIX matrix, MATRIX lower, MATRIX upper, const int size){
	int row = 0; /* row counter */
	int column = 0; /* column counters */
	int counter = 0, counter2 = 0; /* general use loop counters */
	int newRow = -1; /* index of new row, for swapping rows */
	double coeff = 0.0; /* coefficient for multiply a row with */

	MATRIX copy=myMalloc(size);

	copyMatrix(copy,matrix,size);
	/* Prepare lower and upper triangle matrixes for worst case */
	for (row = 0; row < size; ++row) {
		for (column = 0; column < size; ++column) {
			lower[row][column] = 0.0;
			upper[row][column] = 0.0;
		}
	}

	row = 0;
	column = 0;

	for (row = 0; row < size; ++row, ++column) {

		/* If current row starts with a zero */
		if (copy[row][column] == 0.0) {
			newRow = -1;

			for (counter = row + 1; counter < size; ++counter)
				if (copy[counter][column] != 0) {
					newRow = counter;
					break;
				}

			/* swap rows if newRow found*/
			if (newRow != -1)
				swapRows(copy, size, row, newRow);

		}

		/* Make zero below current column */
		for (counter = row + 1; counter < size; ++counter) {
			coeff = (copy[counter][column] / copy[row][column]);

			for (counter2 = column; counter2 < size; ++counter2)
				copy[counter][counter2] -= (copy[row][counter2] * coeff);

			/* from definition of LU decomposition */
			copy[counter][column] = coeff;
		}

		/* Generate lower and upper triangle matrixes */
		for (counter = 0; counter < size; ++counter) {
			/* lower triangle matrix main diagonal is always 1 */
			lower[counter][counter] = 1.0;

			/*fill the rest of lower triangle matrix */
			for (counter2 = (counter + 1); counter2 < size; ++counter2)
				lower[counter2][counter] = copy[counter2][counter];

			/* 
			 * upper triangle matrix main diagonal 
			 * is same with original matrix
			 */
			upper[counter][counter] = copy[counter][counter];
			/*fill the rest of upper triangle matrix */
			for (counter2 = (counter - 1); counter2 >= 0; --counter2)
				upper[counter2][counter] = copy[counter2][counter];

		}
	}
	myFree(copy,size);
	return;
}
/*replace row1 and row2 of given matrix*/
void swapRows(MATRIX matrix, const int row1, const int row2, const int size){
	double* tmp=(double*)malloc(size*sizeof(double));

	/* at least one of rows out of matrix */
	if ((row1 >= size) || (row2 >= size))
		return;
	/* there is no negative index at rows */
	if ((row1 < 0) || (row2 < 0))
		return;

	/* swap rows*/
	copyLine(tmp,matrix[row1],size);
	copyLine(matrix[row1],matrix[row2],size);
	copyLine(matrix[row2],tmp,size);
	free(tmp);
}
/*copy given original matrix line to matrix copy--*/
void copyLine(double copy[MAXSIZE],double original[MAXSIZE],const int size){
	int counter;
	for (counter = 0; counter < size; ++counter)
	{
		copy[counter]=original[counter];
	}
}
/*------end of project2 main part---------*/