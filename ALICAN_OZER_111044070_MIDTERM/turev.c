/*--------------turev.c-----------------------*/
/*--SYSTEM PROGRAMMING MIDTERM PROJECT-----------*/
/*Author:Alican OZER-----------------------------*/
/*Date:14/04/2014  04:00 AM----------------------*/
/*---DERIVATE-CLIENT-PROGRAM---------------------*/

#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MYLOG "LogFileM.txt"
#define MAX_BUF 1024

char MYFIFO[64];
char DERIVATIVE[20];
char BUF[MAX_BUF];
int logFileDescInt;
int fileDesc;
FILE *logging;
long int pidServer;//bağlanılan serverin pidi
long int pidChild;//child pidimiz

void sigHandler(int signo);
int main(int argc, char **argv)
{
    
    system("clear");
    if (argc != 3)
    {
        printf("HATALI PARAMETRE DiZiNi\n");
        printf("USAGE: %s <serverPID> <time(milisec)>\n", argv[0]);
        exit(0);
    }

    if(atol(argv[1]) < 0 || atol(argv[2]) < 0)
    {
        printf("HATALI PARAMETRE DiZiNi\n");
        printf("USAGE: %s <serverPID> <time(milisec)>\n", argv[0]);
        exit(0);
    }
    
    strcpy(MYFIFO,argv[1]);//fifo pid kontrolu
    if((fileDesc=open(MYFIFO, O_WRONLY)) == -1)
    {
        printf("No such pid-server:%s\n",MYFIFO);
        exit(0);
    }
    
    char pName[MAX_BUF];
    memset(pName,' ',MAX_BUF);
    strcpy(pName,"1");
    snprintf(pName+1,MAX_BUF-1,"%10ld",(long)getpid());
    snprintf(pName+11,MAX_BUF-11,"%s",argv[2]);//turev belirteci
    
    /*0:FONKSİYON TÜRÜ + 10:pid +SÜRE*/
    write(fileDesc,pName,MAX_BUF);
    close(fileDesc);

/*----------------------------------------------*/
        
    char pidTemp[10];
    snprintf(pidTemp,10,"%ld",(long)getpid());
    snprintf(DERIVATIVE,20,"%ld-DERIVATIVE",(long)getpid());
    mkfifo(DERIVATIVE, 0666);
    logFileDescInt = open(DERIVATIVE,O_RDONLY);    

    if ((logging=fopen(MYLOG,"a")) == NULL)
        {
            printf("Cannot open logFile:%s\n",MYLOG );
            exit(0);
        } 
    printf("RUNNiNG...\n");
    while(1)
    {
        while( read(logFileDescInt,BUF,MAX_BUF) > 0 ){
            signal(SIGINT, &sigHandler);
            if(!strncmp(BUF,"FINISHED",8)){
                printf("DERIVATIVE Completed [%s]\n",BUF);
                close(logFileDescInt);
                unlink(DERIVATIVE);
                fclose(logging);
                exit(1);
            }//serverdan gelen veriler fila yazılır
            sscanf(BUF,"%ld %ld",&pidServer,&pidChild);
            fprintf(logging, "%s",BUF );
        }
    }
    return 0;
}
void sigHandler(int signo){
    kill(pidChild,SIGINT);
    close(logFileDescInt);
    unlink(DERIVATIVE);
    printf("===Client CTRL-C caught and exited===\n");
    fclose(logging);
    exit(signo);
}
/*----------end of 111044070 turev.c -------------------*/