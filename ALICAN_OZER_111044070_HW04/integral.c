/*-----------integral.c--------------
*--SYSTEM PROGRAMMING HW04-----------
*Author:Alican OZER------------------
*Date:29/04/2014  05:00 AM-----------
*---INTEGRAL-CLIENT-PROGRAM----------
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
char charPtrFifoInt[20];
char BUF[MAX_BUF];
int intLogFileInt;
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
    memset(charPtrSet,' ',MAX_BUF);//IIINI BOŞALT
    strcpy(charPtrSet,"2");//İNTEGRAL BELİRTECİ
    snprintf(charPtrSet+1,MAX_BUF-1,"%10d",getpid());
    snprintf(charPtrSet+11,MAX_BUF-11,"%s",argv[2]);//İNTEGRAL belirteci
    
    /*0:FONKSİYON TURU + 10:pid +SURE*/
    write(intFifo,charPtrSet,MAX_BUF);//servera yazma bitti
    close(intFifo);
    printf("MYPID:%d\n",getpid() );
/*----------------------------------------------*/
        
    snprintf(charPtrFifoInt,20,"%d-INTEGRATE",getpid());
    mkfifo(charPtrFifoInt, 0666);
    intLogFileInt = open(charPtrFifoInt,O_RDONLY);    

    time_t currentTime;//zaman değişkenleri
    struct tm *myTime;
    currentTime = time( NULL );
    myTime = localtime( &currentTime );

    snprintf(charLogFile,20,"INTEGRAL%d.log",getpid());
    printf("INTEGRAL-charLogFile:%s\n",charLogFile);
	if((filePtrLogFile=fopen(charLogFile,"w")) != NULL){
    	fprintf(filePtrLogFile,"[Integral:%d] Starting Time is %.2d:%.2d:%.2d\n",
    		getpid(),myTime->tm_hour, myTime->tm_min, myTime->tm_sec );
        fprintf(filePtrLogFile,"SERVER__CLIENT__dt(ms)__VALUE___RESULT__\n" );
    }
    else {printf("LOG FILE OLUSTURULAMADI\n");  exit(0);}
    
    printf("RUNNiNG...\n");
    while(1)
    {
        while( read(intLogFileInt,BUF,MAX_BUF) > 0 ){
            signal(SIGINT, &handlerSIGSET);
            if(!strncmp(BUF,"FINISHED",8)){
                printf("Integral Completed [%s]\n",BUF);
                close(intLogFileInt);
                unlink(charPtrFifoInt);
                fclose(filePtrLogFile);
                exit(1);
            }//serverdan gelen veriler dosyaya yazilir
            fprintf(filePtrLogFile, "%s",BUF );
        }
    }
    return 0;
}
void handlerSIGSET(int signo){
    close(intLogFileInt);
    unlink(charPtrFifoInt);
    printf("===Client CTRL-C caught and exited===\n");
    fclose(filePtrLogFile);
    exit(signo);
}
/*----------end of 111044070 integral.c -------------------*/