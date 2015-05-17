/*********************************************
*   CSE-244 SYSTEM PROGRAMMING FALL-2014
*   Alican OZER
*   111044070
*   alicanozer60@gmail.com
******************************
*   Description:
*   This homework is a local server program,
*   that creats number of thread and executes them.
*/

/**********************************************/
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>

/**********************************************/
#define SEMNAME "/ozer"
#define SHMPERMS (IPC_CREAT | 0666)
#define SHMKEY 111044070
#define MINPOINT 0
#define MAXPOINT 1
#define MAXCLIENT 20
#define MAXLEN 1024
#define MAX_TOKEN 256//maximum string boyutu
#define TEN_BILLION 10000000L
/**********************************************/
    struct Data{
        double whellOfFunc[MAXLEN];
        int arrSize;
        int randNum;
        int pidClient; 
        int waitForServer;     
	};    
/**********************************************/    
    void printAverage(long int timeNow);
    void sigHandler(int signo);
/**********************************************/

    struct Data *serverData;
    int shmID=0;
    sem_t *semServer;
    FILE *mainLog;
    int clientPIDs[MAXCLIENT];
    int numOfClient=0;
    int readyClient=0;
    int bufSize=0;
    struct timespec sleeptime;

/**********************************************/
int main(int argc, char const *argv[])
{	
    signal(SIGINT,sigHandler);	
    
    struct timeval tvalStart,tvalBefore, tvalAfter;/*zaman farki icin*/	
    time_t currentTime;/*zaman degiskenleri*/
    struct tm *timePtrTime;
    int avCount=1;
    int diffTime=0,updateTime=0,randomNumber=0;
    int count=0;
    char myLog[30];
    
    sleeptime.tv_sec=0;
    sleeptime.tv_nsec=TEN_BILLION;    
    
    system("clear"); 
    if (argc != 3)
	{
		printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
		printf("USAGE>$ %s_<bufSize> <updateTime>\n",argv[0] );
		exit(0);
	}
	
	bufSize=atoi(argv[1]);/*********/
    updateTime=atoi(argv[2]);

    if (bufSize < 1 || updateTime < 1)
    {
        printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
        printf("USAGE>$ %s_<bufSize>_<updateTime>\n",argv[0] );
        printf("!!!PLEASE ENTER<POSITIVE>NUMBER AS ARGUMENTS!!!\n");
        exit(0);
    }
    srand(time(NULL));
    randomNumber=rand()%bufSize;    

    snprintf(myLog,30,"MSERVER-%d.log",getpid());
    mainLog=fopen(myLog,"w");
    printf("bufSize:%d\tupdateTime:%d milisec.\n",bufSize,updateTime );
    fprintf(mainLog,"bufSize:%d\tupdateTime:%d milisec.\n",bufSize,updateTime );   
    
    /* initialize the semaphore */
    semServer=sem_open(SEMNAME, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR , 0666);    
    if(semServer != SEM_FAILED )
        printf("created new semaphore !\n");
    else if(errno == EEXIST ){ 
        printf("semaphore appears to exist already !\n");
        sem_close(semServer);
        sem_unlink(SEMNAME);
        semServer=sem_open(SEMNAME, O_CREAT | O_EXCL , S_IRUSR | S_IWUSR , 0666);
        if(semServer != SEM_FAILED )
            printf("created new semaphore !\n");
        else return 1;
    }  
       
    
	if((shmID = shmget((key_t)SHMKEY,sizeof(struct Data),SHMPERMS)) == -1){/*creat*/
		perror("[main]Failed to create shared memory segment");
		return 1;
	}

	if((serverData = (struct Data*)shmat(shmID,NULL,0)) == (void*)-1){/*attack*/
		perror("[main]Failed to attach shared memory segment");
		if (shmctl(SHMKEY, IPC_RMID, NULL) == -1)
			perror("[main-error]Failed to remove memory segment");
		return 1;
	}

    for (count = 0; count < bufSize; ++count){
        serverData->whellOfFunc[count]=(count+1)*((double)(MAXPOINT-MINPOINT))/(bufSize+1);
 //       printf("%f\n",serverData->whellOfFunc[count] );
    }
/*-------------------------------------*/
    serverData->arrSize=bufSize;
    serverData->randNum=randomNumber;
    serverData->pidClient=0;
    serverData->waitForServer=1;
/*-------------------------------------*/
    gettimeofday (&tvalBefore, NULL);
    gettimeofday (&tvalStart, NULL);
     
    currentTime = time( NULL );
    timePtrTime = localtime( &currentTime );
    /*----------------------------*/
    fprintf(mainLog,"MServer[Server] Started %.2d:%.2d:%.2d\n",
        timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec );
    fprintf(mainLog,"----------------------------------------\n");
    fprintf(mainLog,"-AVERAGE--&--TIME---\n");

    printf("running.over.%s\n",SEMNAME);

    /* run until catch ctrl-c */ 
    while(1){           
        	
        if (serverData->pidClient > 0)
        {
            printf("Client Connected...[pid:%d]\n",serverData->pidClient);
            clientPIDs[numOfClient] = serverData->pidClient;
            numOfClient++;
            readyClient++;
            serverData->pidClient=0;
        }else if (serverData->pidClient < 0){
            printf("Client Disconnected......\n");
            readyClient--;
            serverData->pidClient=0;
        }
        gettimeofday(&tvalAfter,NULL);
        diffTime=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 + 
        	(tvalAfter.tv_usec - tvalBefore.tv_usec )/1000);
        
        if(diffTime >= updateTime){
            serverData->waitForServer=0;
	        printAverage(avCount*updateTime); 
	        avCount++;              
	        gettimeofday (&tvalBefore, NULL);
	        diffTime=0;
            serverData->waitForServer=1;
    	}
    	/* release a thread to execute */
    	nanosleep(&sleeptime,NULL);            
    	sem_post(semServer);
    }    
    return 0;
}

/*******************************************************/
void sigHandler(int signo){

	int count=0,c2=0;
	
    printf(":::MSERVER ctrl-c CATCHED:PLEASE WAIT FOR CLIENTS CLEANING::\n");    
    fprintf(mainLog,":::MSERVER ctrl-c CATCHED:::\n");     
    fclose(mainLog);       

    for (count = 0; count < numOfClient; ++count){    
        kill(clientPIDs[count],SIGINT);
        for(c2=0;c2<MAX_TOKEN;c2++){                        
    	    sem_post(semServer);
    	    nanosleep(&sleeptime,NULL);
	    }    
    }  
    
    sem_close(semServer);
    shmdt((void*) serverData);
    
    
    sleep(readyClient*4);
    
    sem_unlink(SEMNAME);
    if (shmctl(shmID, IPC_RMID, NULL) == -1)
       perror("[signal]Failed to remove memory segment");
       
	exit(signo);
}
/*----------------------------*/
void printAverage(long int timeNow)
{
    double sum=0.0;
    int count=0;

    for (count = 0; count < bufSize; ++count)
        sum += serverData->whellOfFunc[count]; 

    fprintf(mainLog,"%f\t%ld\n",sum/bufSize,timeNow);
 //   printf("%f\t%ld\n",sum/bufSize,timeNow);
    srand(time(NULL));
    serverData->randNum=rand()%bufSize;
}
