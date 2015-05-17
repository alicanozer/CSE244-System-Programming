/*--------------ALICAN_OZER_111044070.c--------------------------*/
/*----------SYSTEM_PROGRAMMING_CSE_244_HW02----------------------*/
/*DATE:--17/03/2014----------------------------------------------*/
/*AUTHOR:ALiCAN_OZER---------------------------------------------*/
/*---------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#define OUTPUTFILE "TEMPFILE_OZER.txt"

/*------------------------------------------*/
/*REC 0 ISE VERILEN KLASORDEKI DOSYALARI BULUR VE PARAMETREDEKI STRINGE ATAR(REC 1 ISE)*/
/*VE BULDUGU DOSYA SAYISINI DONDURUR*/
int numOfFileOfDir(const int REC,char *dirName,char **files,int used,int *numOfFile);
/*VERILEN DOSYANIN EN UZUN SATIRININ BOYUTUNU DONDURUR*/
int findMaxSize(char fileName[]);
/*HW1 DE KULLANDIGIM ARAMA FONKSIYONU DOSYAADI VE TEXT ALIR */
void mGrep(int size,char fileName[],char key[]);
/*mGREP IN BULDUGU YAZILARI DOSYADAN OKUR ISTENILEN SAYI KADAR BASAR*/
void printAll(int maxSize,int request);
/*HOCANIN ISTEDIGI FONKSIYON TUM ARGUMANLARI ALIR*/
int bulbeni(char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,int arg5);
/*------------------------------------------*/

int main(int argc, char *argv[]){
    int LINE;

	if(argc != 6) {   
	printf("Usage:%s DIR -g WORD -l LineNO(unsigned)\n-OR-  %s DIR -l LineNO(unsigned) -g WORD\n",argv[0],argv[0]);
		return 0;
		}
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
	printf("Usage:%s DIR -g WORD -l LineNO(unsigned)\n-OR-  %s DIR -l LineNO(unsigned) -g WORD\n",argv[0],argv[0]);}

	return 1;
/**************ANA FONKSIYON***********************/	
}
int bulbeni(char *arg0,char *arg1,char *arg2,char *arg3,char *arg4,int arg5){
	pid_t pid;
	char **myFiles;/*TUM DOSYA ISIMLERINI DEPOLAR*/
	int numOfFile=0;/*STRINGE ATMA SIRASINI TUTAR*/
	int i=0,j=0,size=0;

/*SADECE DOSYA SAYISINI OGRENMEK ICIN*/
	size=numOfFileOfDir(0,arg1,NULL,0,&numOfFile);
	myFiles=(char **)malloc(size*sizeof(char*));
   	for(i=0;i<size;i++)										
    	myFiles[i]=(char *)malloc(100*260*sizeof(char));
 
    numOfFile=0;
/*TUM KLASORLERI VE DOSYALARI TARAR ADRESLERINI STRINGE KAYDEDER*/
    size=numOfFileOfDir(1,arg1,myFiles,0,&numOfFile);
    printf("TARANAN DOSYA SAYISI::%d\n",size );

printf("##########################---DOSYALAR VE PID LERI---########################\n");
	for(i=0;i<size;i++)
	{
		if ((pid = fork()) == -1) {
			perror("FORK UNCOMPLETED..\n");
			exit(1);
		}
		if (pid == 0) {
			j=findMaxSize(myFiles[i]);
			mGrep(j,myFiles[i],arg3);
printf("SAYI::%-4d**PID:%5ld**%-67s\n",i+1, (long)getpid(),myFiles[i]);
			exit(0);			
		}
		else {
		wait(NULL);
		}
	}
printf("#############################---ARAMA-SONUCLARI---##########################\n");

printAll(100*260,arg5);
/*ALANLARI BOSALTMA*/
   	for(i=0;i<size;i++)										
    	free(myFiles[i]);
    free(myFiles);
	return 1;
}
/*------------------------------------------*/
int numOfFileOfDir(const int REC,char *dirName,char **files,int used,int *numOfFile){
	DIR *mainFolder;
	char *temp=(char*)malloc((100*260)*sizeof(char));
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
		exit(0);
	}
	free(temp);
	return used;/*BULDUGU DOSYA SAYISI*/
}
/*------------------------------------------*/
int findMaxSize(char fileName[]){
	FILE *inputFile;
	char test;	  /*DOSYADAN OKUNACAK KARAKTER*/
	int st;      /*DOSYA SONUNA ULASILMA DURUMU*/
	int size=0; /*DONDURULECEK UZUNLUK*/
	int def=0;  /*GECICI UZUNLUK*/
	if((inputFile=fopen(fileName,"r"))==NULL)
	{
		printf("%s_ACILAMADI\n",fileName);
		return 0;
	}
	while(fscanf(inputFile,"%c",&test) != EOF){
		if(test == '\n'){
			def++;
			if(size < def)
				size = def;
		}/*BUYUK OLAN UZUNLUK TUTULUR*/
		else def++;
	}
	fclose(inputFile);
	return size;
}
/*------------------------------------------*/
void mGrep(int size,char fileName[],char key[])
{
	FILE *fileP;
	char *line = (char *)malloc(size * sizeof(char));
	char *splitter;
	int outer=1;    /*SATIR SAYICI*/
	int inner=0;    /*ELEMAN SAYICI*/
	FILE *outputFile;
	if((outputFile=fopen(OUTPUTFILE,"a"))==NULL)
	{
		printf("(PID:%ld)_%s_DOSYASI_OLUSTURULAMADI\n",(long)getpid(),OUTPUTFILE);
		return;
	}	
	if((fileP=fopen(fileName,"r"))==NULL)
	{
		printf("(PID:%ld)_%s_DOSYASI_ACILAMADI\n",(long)getpid(),fileName);
		return;
	}
	while (fgets(line, size, fileP) != NULL ) 
	{		
		splitter = strstr(line,key);			
		while(splitter != NULL)
		{		
			fprintf(outputFile,"%5d.SATIRDA**(PID:%ld)./%s\n",outer,(long)getpid(),fileName);
			inner++;    /*TOPLAM BULMA SAYISI*/			
			splitter=strstr(splitter+1,key);/*AYNI SATIRDA BIRDEN FAZLA OLMA DURUMU*/
		}
		outer++;
	}	
	free(line);
	if(inner != 0)
		fprintf(outputFile,"(PID:%ld)./%s_---------------TOPLAM:%3d\n",(long)getpid(),fileName,inner);
	
	fclose(fileP);
	fclose(outputFile);
}
/*------------------------------------------*/
void printAll(int maxSize,int request){

	FILE *inputFile;
	int totalCount=1;
	char a;
	char *line=(char*)malloc(maxSize*sizeof(char));
	int count=0;

	if((inputFile = fopen(OUTPUTFILE,"r")) == NULL)
	{
		printf("%s_ACILAMADI\n",OUTPUTFILE);
		return;
	}
	if(request==0){
		free(line);
		fclose(inputFile);
		remove(OUTPUTFILE);
		printf("iSTEK 0 :HIC BISEY YAZDIRILMADI\n");
		return;
	}

	while (fgets(line, maxSize, inputFile) != NULL)
	{	/*ISTEK KADAR YAZDIRIR*/	
		if(count<request){
			printf("%2d:%s",totalCount,line);
			totalCount++;	
		}
		else if(count==request)
		{
			printf("\n--Less--(Q or q EXIT)--Less--");
			scanf("%c",&a);
			if(a=='Q' || a=='q')
			{
				free(line);
				fclose(inputFile);
				if(remove(OUTPUTFILE) != 0) 
				fprintf(stderr, "Error deleting file %s.\n", OUTPUTFILE);
				return;
			}
			else count = -1;
		}	
		count++;
	}
	free(line);
	fclose(inputFile);
	if(remove(OUTPUTFILE) != 0) /*URETILEN DOSYAYI SILME*/
		fprintf(stderr, "Error deleting file %s.\n", OUTPUTFILE);
}
/*-----------------END_OF_ALICAN_OZER_111044070.c-------------------------*/
