/*--------------ALICAN_OZER_111044070.c--------------------------*/
/*----------SYSTEM_PROGRAMMING_CSE_244_HW03----------------------*/
/*DATE:--02/04/2014----------------------------------------------*/
/*AUTHOR:ALiCAN_OZER---------------------------------------------*/
/*---------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#define MAXLEN 200000 /*MAXIMUM SATIR UZUNLUGU*/
int totalCount=0; /*TOPLAM BULGU SAYISI*/
int st=0; /*Q TRAP STATUS*/

void ctrlcChild(int sig1);
void printAll(int *FD,int request);
void mGrep(int *FD,char fileName[],char key[]);
int bulbeni(char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,int arg5);

int main(int argc, char *argv[]){
	int LINE=0;
	system("clear");
	if(argc != 6) {   
	printf("Usage:%s DIR -g WORD -l LineNO(unsigned)\n-OR-  %s DIR -l LineNO(unsigned) -g WORD\n",argv[0],argv[0]);
	return 0;}
	if(!strcmp(argv[2],"-g") && !strcmp(argv[4],"-l"))
	{
		LINE=atoi(argv[5]);
		if(LINE <= 0) LINE = (-1)*LINE;
		return bulbeni(argv[0],argv[1],argv[2],argv[3],argv[4],LINE);
	}
	else if(!strcmp(argv[4],"-g") && !strcmp(argv[2],"-l"))
	{
		LINE=atoi(argv[3]);
		if(LINE <= 0) LINE = (-1)*LINE;
		return bulbeni(argv[0],argv[1],argv[4],argv[5],argv[2],LINE);
	}
	else {		
		printf("%s %s %s %s %s %s", argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]);
		printf("_DESTEKLENMEYEN_FORMAT_\n");
		printf("Usage:%s DIR -g WORD -l LineNO(unsigned)\n-OR-  %s DIR -l LineNO(unsigned) -g WORD\n",argv[0],argv[0]);
		}
	return 1;
}
/**************ANA FONKSIYON***********************/
int bulbeni(char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,int arg5){
	char **myFiles;/*TUM DOSYA ISIMLERINI DEPOLAR*/
	int numOfFile=0;/*STRINGE ATMA SIRASINI TUTAR*/
	int i=0,size=0;
	int fd[2];
	pid_t pid;

/*SADECE DOSYA SAYISINI OGRENMEK ICIN*/
	size=numOfFileOfDir(0,arg1,NULL,0,&numOfFile);
	myFiles=(char **)malloc(size*sizeof(char*));
   	for(i=0;i<size;i++)										
    	myFiles[i]=(char *)malloc(MAXLEN*sizeof(char));
 
    numOfFile=0;
/*TUM KLASORLERI VE DOSYALARI TARAR ADRESLERINI STRINGE KAYDEDER*/
    size=numOfFileOfDir(1,arg1,myFiles,0,&numOfFile);
    printf("===DiZiNDE TARANAN DOSYA SAYISI:%d===\n",size );
	
	for(i=0;i<size;i++)
	{
		signal(SIGINT, &ctrlcChild);/*handler*/
		if(pipe(fd) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }	
	  	if ((pid=fork()) == 0) {
		    if(st) exit(0);/*q durumu*/
		    close (fd[0]); /* kullanılmayan çıkışı kapat*/
/*her seferinde bir dosya gönderilir ve sonuclar pipe a yazılır*/		    		 	
		    mGrep(&fd[1],myFiles[i],arg3);
		    close (fd[1]); /* kullanılan çıkışı kapat*/
		  	exit(0);
		   
	  	} 
	  	else {
	  		close(fd[1]);/* kullanılmayan çıkışı kapat*/
	  		if(st) break;/*q basıldıysa prosesler ölür*/
	  		printAll(&fd[0],arg5);/*pipe dan okuma*/  		
			close (fd[0]); /* kullanılan çıkışı kapat*/
	  	} 
   }
	if(st)
	printf("!!!ARAMA DURDURULDU %d TANE GOSTERILDI\n",totalCount );
  	else printf("---TUM DOSYALARDA TOPLAM %d TANE VAR---\n",totalCount );

   	for(i=0;i<size;i++)	/*dosya isimlerinin bulunduğu alan serbest*/							
    	free(myFiles[i]);
    free(myFiles);
	exit(1);
}
void mGrep(int *FD,char fileName[],char key[]){
	char *line=(char*)malloc(MAXLEN*sizeof(char));
	FILE *fileP;	
	char info[MAXLEN]="";
	char *splitter;
	int outer=1;    /*SATIR SAYICI*/
	int inner=0;    /*ELEMAN SAYICI*/
	if((fileP=fopen(fileName,"r"))==NULL)
	{
		printf("(PID:%ld)_%s_DOSYASI_ACILAMADI\n",(long)getpid(),fileName);
		free(line);
		return;
	}
	while (fgets(line, MAXLEN, fileP) != NULL ) 
	{		
		splitter = strstr(line,key);/*text bulma*/		
		while(splitter != NULL)/*bulunduysa aynı satırda yine varmı*/
		{
			inner++;    /*TOPLAM BULMA SAYISI*/	
			strcpy(info,"");
			sprintf(info,"=>line:%-6d=>pid:%-5ld=>total:%-4d in %s\n",outer,(long)getpid(),inner,fileName);
			write (*FD, info, MAXLEN +1);	
			splitter=strstr(splitter+1,key);/*AYNI SATIRDA BIRDEN FAZLA OLMA DURUMU*/
		}
		outer++;
	}	
	free(line);
	fclose(fileP);
}
/*------------------------------------------*/
void printAll(int *FD,int request){
	char a='a';/*çıkış kontrolü için*/
	static int count=0;/*sadece ilk girişte tanımlanır*/
	char *line=(char*)malloc(MAXLEN*sizeof(char));
	if(request==0){
		free(line);
		printf("iSTEK 0 :HIC BISEY YAZDIRILMADI\n");
		exit(0);
	}
	while (read(*FD, line, MAXLEN+1) > 0)
	{
		if(count==request)/*terminalden girilen sayı sayaca eşitmi*/
		{
			printf("\n--Less--(Q or q EXIT)--Less--");
			scanf("%c",&a);
			if(a=='Q' || a=='q')
			{
				free(line);
				st=1; /*diğer proseslere haber verilir ve çıkılır*/
				return;
			}
			else count = 0;
			printf("\n");
		}
			totalCount++;
			count++;
			printf("%4d)(parentID %-5ld)%s",totalCount,(long)getpid(),line);
	}
	free(line);
	return;
}

void ctrlcChild(int sig1){
    printf("\n=>Pressed Ctrl-C Process:%ld Terminated!(%d)\n",(long)getpid(),sig1);
    exit(sig1);
}
int numOfFileOfDir(const int REC,char *dirName,char **files,int used,int *numOfFile){
	DIR *mainFolder;
	char *temp=(char*)malloc((MAXLEN)*sizeof(char));
	struct dirent *openedFile;
	if ((mainFolder = opendir(dirName)) != NULL) 
	{
		while ((openedFile = readdir(mainFolder)) != NULL) 
		{	
			if (openedFile->d_type == DT_REG)/*SIMPLE CASE DOSYA OKUMA*/
			{		
				strcpy(temp,dirName);
				strcat(temp,"/");
				strcat(temp,openedFile->d_name);
				used++;
				if (REC)
				strcpy(files[(*numOfFile)],temp);				
				(*numOfFile)++;
			}
			if(openedFile->d_type == DT_DIR )/*RECORSION CASE KLASOR GELIRSE*/
			{
				if(strcmp(openedFile->d_name,".") && strcmp(openedFile->d_name,".."))
				{
					strcpy(temp,dirName);
					strcat(temp,"/");
					strcat(temp,openedFile->d_name);
					used += numOfFileOfDir(REC,temp,files,0,numOfFile);
				}
			}					
		}
		closedir(mainFolder);/*KLASOR KAPATMA*/
	}
	else 
	{
		printf("%s_ACILAMADI\n",dirName);/*KLASOR ACILMASSA*/
		free(temp);
		return used;
	}
	free(temp);
	return used;/*BULDUGU DOSYA SAYISI*/
}
/*--------end of ALICAN_OZER_111044070.c -----------------*/