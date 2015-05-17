/**-----------turev.c----------------
*--SYSTEM PROGRAMMING HW04-----------
*Author:Alican OZER------------------
*Date:29/04/2014  05:00 AM-----------
*---DERIVATE-CLIENT-PROGRAM----------
**/

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_BUF 1024

char charLogFile[20];
char charPtrFifo[64];
char charPtrFifoDer[20];
char charPtrFifoBuffer[MAX_BUF];
int intLogFileDer;
int intFifo;
FILE *filePtrLogFile; 

void handlerSIGSET(int signo);
int main(int argc, char **argv)
{
    
    system("clear");
    if (argc != 3)
    {
        printf("HATALI PARAMETRE DiZiNi\n");
        printf("USAGE: %s <serverPID> <time(milisec)>\n", argv[0]);
        exit(0);
    }

    if(atoi(argv[1]) < 0 || atol(argv[2]) < 0)
    {
        printf("HATALI PARAMETRE DiZiNi\n");
        printf("USAGE: %s <serverPID> <time(milisec)>\n", argv[0]);
        exit(0);
    }
    
    strcpy(charPtrFifo,argv[1]);//fifo pid kontrolu
    if((intFifo=open(charPtrFifo, O_WRONLY)) == -1)
    {
        printf("No such pid-server:%s\n",charPtrFifo);
        exit(0);
    }
    
    char charPtrSet[MAX_BUF];
    memset(charPtrSet,' ',MAX_BUF);
    strcpy(charPtrSet,"1");
    snprintf(charPtrSet+1,MAX_BUF-1,"%10d",getpid());
    snprintf(charPtrSet+11,MAX_BUF-11,"%s",argv[2]);//turev belirteci
    
    /*0:FONKSIYON TURU + 10:pid +SURE*/
    write(intFifo,charPtrSet,MAX_BUF);
    close(intFifo);
printf("MYPID:%d\n",getpid() );
/*----------------------------------------------*/
        
    snprintf(charPtrFifoDer,20,"%d-DERIVATIVE",getpid());
    mkfifo(charPtrFifoDer, 0666);
    intLogFileDer = open(charPtrFifoDer,O_RDONLY);    

    time_t currentTime;//zaman değişkenleri
    struct tm *myTime;
    currentTime = time( NULL );
    myTime = localtime( &currentTime );

    snprintf(charLogFile,20,"TUREV%d.log",getpid());
    printf("TUREV-charLogFile:%s\n",charLogFile);
    if((filePtrLogFile=fopen(charLogFile,"w")) != NULL){
    	fprintf(filePtrLogFile,"[Turev:%d] Starting Time is %.2d:%.2d:%.2d\n",
    		getpid(),myTime->tm_hour, myTime->tm_min, myTime->tm_sec );
        fprintf(filePtrLogFile,"SERVER__CLIENT__dt(ms)__VALUE__RESULT__\n" );
    }
    else {printf("LOG FILE OLUSTURULAMADI\n");  exit(0);}
    
    printf("RUNNiNG...\n");
    while(1)
    {
        while( read(intLogFileDer,charPtrFifoBuffer,MAX_BUF) > 0 ){
            signal(SIGINT, &handlerSIGSET);
            if(!strncmp(charPtrFifoBuffer,"FINISHED",8)){
                printf("Turev Completed [%s]\n",charPtrFifoBuffer);
                close(intLogFileDer);
                unlink(charPtrFifoDer);
                fclose(filePtrLogFile);
                exit(1);
            }//serverdan gelen veriler file yazilir
            fprintf(filePtrLogFile, "%s",charPtrFifoBuffer );
        }
    }
    return 0;
}
void handlerSIGSET(int signo){    
    close(intLogFileDer);
    unlink(charPtrFifoDer);
    printf("===Client CTRL-C caught and exited===\n");
    fclose(filePtrLogFile);
    exit(signo);
}
/*----------end of 111044070 turev.c -------------------*/