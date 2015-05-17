/*-----------------ALICAN_OZER_111044070.c---------------------------*/
/*LESSON: SYSTEM PROGRAMMING HOMEWORK 1:GREP COMMAND-----------------*/
/*DATE  : 10/03/2014-------------------------------------------------*/
/*AUTHOR: ALiCAN OZER------------------------------------------------*/
/*USAGE: ./exe inputfile word (enter)--------------------------------*/
/*********************************************************************/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Bu fonksiyon dosyadaki en uzun satırı bulup onun uzunluğunu döndürür*/
int findMaxSize(FILE *inputFile,char fileName[]);
void mGrep(int size,char fileName[],char key[]);

int main(int argc, char *argv[]) {
	FILE* fileP;
	int size=0;
	if (argc != 3) {    /*HATALI PARAMATRE GIRISI*/
		fprintf(stderr, "Usage: %s filename text\n", argv[0]);
		return 0;
	}   /*YANLIS YADA OLMAYAN DOSYA*/
	if ((fileP = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "OKUMA HATASI: %s>%s\n", argv[1], strerror(errno));
		return 0;	
	}
	else printf("ACCESSED TO FILE:%s\n",argv[1]);
	fclose(fileP);  /*fonksiyonda acılacak zaten*/
	size=findMaxSize(fileP,argv[1]);
	mGrep(size,argv[1],argv[2]);	
return 0;
}

int findMaxSize(FILE *inputFile,char fileName[]){
	char test;	  /*DOSYADAN OKUNACAK KARAKTER*/
	int st;      /*DOSYA SONUNA ULASILMA DURUMU*/
	int size=0; /*DONDURULECEK UZUNLUK*/
	int def=0;  /*GECICI UZUNLUK*/
	inputFile=fopen(fileName,"r");
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
void mGrep(int size,char fileName[],char key[])
{
	FILE *fileP;
	char *line = (char *)malloc(size * sizeof(char));
	char *splitter;
	int outer=1;    /*SATIR SAYICI*/
	int inner=0;    /*ELEMAN SAYICI*/
	fileP=fopen(fileName,"r");
	while (fgets(line, size, fileP) != NULL ) 
	{		
		splitter = strstr(line,key);			
		while(splitter != NULL)
		{		
			printf("LINE_NO:%5d\n",outer);
			inner++;    /*TOPLAM BULMA SAYISI*/			
			splitter=strstr(splitter+1,key);/*AYNI SATIRDA BIRDEN FAZLA OLMA DURUMU*/
		}
		outer++;
	}	
	free(line);
	if(inner != 0)
		printf("=============\n#TOPLAM:%5d \n",inner);
	else printf("!!!>%s is not in %s\n",key,fileName);
	fclose(fileP);
}
/*-----------end of ALICAN_OZER_11104070.c-----------------------------*/