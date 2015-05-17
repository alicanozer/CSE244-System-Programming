/*********************************************
*	CSE-244 SYSTEM PROGRAMMING FALL-2014
*	Alican OZER
*	111044070
*	alicanozer60@gmail.com
******************************
*	Description:
*	This homework is a local server program,
*	that creats number of thread and executes them.
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

#include "MClient.h"
static int ISREADY = 1;

int main(int argc, char const *argv[])
{	
	signal(SIGINT,SIGINTHandler);
    char BUFFER[MAX_TOKEN];
	char myLog[30];
	int count=0;
	
	sleeptime.tv_sec=0;
    sleeptime.tv_nsec=TEN_BILLION;
	
/***************************************************/

	if (argc != 2)
	{
		printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
		printf("USAGE>$ %s_<numOfThread>\n",argv[0] );
		exit(EXIT_FAILURE);
	}
	
    numOfThread=atoi(argv[1]);

    if (numOfThread < 1)
    {
        printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
        printf("USAGE>$ %s_<numOfThread>\n",argv[0] );
        printf("!!!PLEASE ENTER<POSITIVE>NUMBER AS ARGUMENTS!!!\n");
        exit(EXIT_FAILURE);
    }
/*****************************************************/

	if((shmID = shmget((key_t)SHMKEY,sizeof(struct Data), SHMPERMS )) < 0){/*creat*/
		perror("[main]Failed to create shared memory segment");
		exit(EXIT_FAILURE);
	}

	if((clientData = (struct Data *)shmat(shmID,NULL,0)) == (void*)-1){/*attack*/
		perror("[main]Failed to attach shared memory segment");
		if (shmctl(shmID, IPC_RMID, NULL) == -1)
			perror("[main-error]Failed to remove memory segment");
		exit(EXIT_FAILURE);
	}
/****************************************************/

	/*initializing named semaphore*/
    semClient=sem_open(SEMNAME , 0666);
    if (semClient == (sem_t*)SEM_FAILED)
    {
    	perror("Server NOT ready ,start server firstly!!!");/**/
    	exit(EXIT_FAILURE);
    }
/******************************************************/  
  
	memset(BUFFER,0,MAX_TOKEN*sizeof(char));
	memset(myLog,0,30*sizeof(char));	

	sprintf(myLog,"MCLIENT-%d.log",getpid());
    if((logFile=open(myLog,O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR)) == -1){/*| O_APPEND*/
    	perror("file open error");
    	exit(EXIT_FAILURE);
    }
    
    sprintf(BUFFER,":::::::::MCLIENT-%d.log:::::::\n",getpid());
    write(logFile,BUFFER ,strlen(BUFFER));

    sprintf(BUFFER,"-------------------------------------\n");
    write(logFile,BUFFER ,strlen(BUFFER));

    sprintf(BUFFER,"<ThreadID>-&--<(x)>----&---<f(x)>------\n");
    write(logFile, BUFFER,strlen(BUFFER));    
    
/*******************************************************/       
    system("clear");
    fileReader();/************/    
    clientData->pidClient=getpid();//send pid to server
         
    for (threadCounter = 0; threadCounter < numOfThread; ++threadCounter)
    {
        
    	if (pthread_create(&myThreads[threadCounter],NULL,threadFunc,NULL) != 0)
    	{
    		perror("[client]thread creating error:");
    		exit(EXIT_FAILURE);
    	}
    }	
	printf("running.over.%s\n",SEMNAME);
	
    while(1);/*waits for ctrl-c */
    
	return 0;
}
void SIGINTHandler(int signal){
	int count=0;
	ISREADY = 0;
	clientData->pidClient = -10;

	printf("...CTRL-C CATCHED EXITING PROPERLY...!!!\n");
    
    for (count = 0; count < numOfThread; ++count){
    	nanosleep(&sleeptime,NULL);
		pthread_detach(myThreads[count]);
		
	}    
	close(logFile);  
    shmdt((void*) clientData);
	sleep(1);		
	sem_close(semClient);
	exit(signal);
}
/*-*********************************/
void *threadFunc(void *args){
  
    struct timeval finished;/*zaman farki icin*/
    double value=0.0;    
    time_t currentTime;/*zaman degiskenleri*/
    char assign[MAX_TOKEN];
    char myX[20];
    char BUFFER[MAX_TOKEN];
    
   

    /* loop forever */
   	while (ISREADY){
        /* wait for semaphore to be signalled */
        sem_wait(semClient);/*begin critical section*/
        if(!ISREADY) {
            sem_post(semClient);
            break;
            }
        if(clientData->waitForServer){
            if (clientData->randNum >= clientData->arrSize - 1)
            	clientData->randNum=0;

            /*clearing segments of string*/
            memset(myX,0,20*sizeof(char));
            memset(assign,0,MAX_TOKEN*sizeof(char));
            memset(BUFFER,0,MAX_TOKEN*sizeof(char));

            /*calculating value*/
            value=clientData->whellOfFunc[clientData->randNum];
            snprintf(myX,20,"%f",value);//x ile degistir
			strcpy(assign,valueWrapper(function,'x',myX));

            /*assign f(x) to x */
			clientData->whellOfFunc[clientData->randNum]=cevirHep(assign,NULL,NULL,NULL);
            gettimeofday (&finished, NULL);
            
            /*read it to logfile*/
            sprintf(BUFFER,"%lu \t %f \t %f \n",pthread_self(),value,
                    clientData->whellOfFunc[clientData->randNum]);
            write(logFile,BUFFER ,strlen(BUFFER));         
            clientData->randNum++;
        }
        else {
            //printf("Main getting average:+/\n");
        }
        nanosleep(&sleeptime,NULL);  
    	sem_post(semClient);/*end of critical section*/
    }
    
 return;

}
/*
*READ EXPRESSION FROM FILE AND ASSIGN IT TO STRNG
*/
void fileReader(){
	char c;
	int i=0,j=0;
	char old[MAX_TOKEN];

	memset(function,0,MAX_TOKEN*sizeof(char));
    memset(old,0,MAX_TOKEN*sizeof(char));

    FILE* input=fopen(FUNCTIONFILE,"r");
    fscanf(input," %s",old);
    fclose(input);
   
/*önceki parsingimde pow yoktu pow-u ekledim*/	
	while(i<strlen(old)){
		if(!strncmp(old+i,"pow(",4)){
			j=i;
			i += 4;
			function[j++]='(';
			
			while(old[i] != ',')
				function[j++]=old[i++];
				
			i++;
			function[j++]=')';
			function[j++]='^';
			function[j++]='(';
	
			while(old[i] != ')')
				function[j++]=old[i++];	
		}		
		else function[j++]=old[i++];	
	}
	printf("Function:::%s\n",old);	
}
/*
*this function replace x form its value
*return new string not included unknown character
*/
char *valueWrapper(const char *first, char pop, const char *push) {
    int count = 0;
    const char *tempForWrapper;//stringin devamını tutmak için

    for(tempForWrapper=first; *tempForWrapper; tempForWrapper++)
        count += (*tempForWrapper == pop);

    size_t rlen = strlen(push);
    char *ptr = turnWrapper;
    for(tempForWrapper=first; *tempForWrapper; tempForWrapper++) {
        if((*tempForWrapper == pop) && (tempForWrapper[1] != 'p')) {
            memcpy(ptr, push, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *tempForWrapper;
        }
    }
    *ptr = 0;
    return turnWrapper;
}
/*çeviri durumda hata gelirse mesaj verir*/
void ceviriHata( parsVeri *pv, const char *err ){
    pv->hata = err;
    longjmp( pv->err_jmp_buf, 1);
}
double cevirHep( const char *expr, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri ){
    double val;
    parsVeri pv;
    dataHazirla( &pv, expr, deger, fonksiyon, myVeri );
    val = baslaCevir( &pv );
    if( pv.hata ){
        printf("Hata: %s\n", pv.hata );
        printf("'%s' çeviri hatası -nan- alındı\n", expr );
        exit(0);
    }
    return val;	
}
/*creating new data type for parsing*/
int dataHazirla( parsVeri *pv, const char *str, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri ){
    pv->str = str;
    pv->fonksiyon = fonksiyon;
    pv->myVeri = myVeri;
    pv->hata = NULL;    
    pv->deger = deger;
    pv->pos = 0;
    pv->len = strlen( str )+1;
    return 1;
}

/*parsing starter */
double baslaCevir( parsVeri *pv ){
    double result = 0.0;
    if( !setjmp( pv->err_jmp_buf ) ){
        result = islemOku( pv ); 
        boslukAtla( pv );
        if( pv->pos < pv->len-1 ){
            ceviriHata( pv, "parsing yaparken hata oluştu" );
        } else return result;
    } else {
        // hata was returned, output a nan silently
        return sqrt( -1.0 );
        }
    return sqrt(-1.0);
}

/*en son okunan karakteri geri döndürür*/
char geriVer( parsVeri *pv ){
    if( pv->pos < pv->len )
    return pv->str[pv->pos];
    ceviriHata( pv, "Dizin okuma hatası!!!" );
    return '\0';
}
/*herhangi bir karakteri atlar*/
char atla( parsVeri *pv ){
    if( pv->pos < pv->len )
    return pv->str[pv->pos++];
    ceviriHata( pv, "Dizin okuma hatası!" );
    return '\0';
}
/*stringdeki tüm boşlukları atlatır*/
void boslukAtla( parsVeri *pv ){
    while( isspace( geriVer( pv ) ) )
    atla( pv );
}
/*eger sayının içinde nokta varsa noktayla beraber okur*/
double doubleOku( parsVeri *pv ){
	char c, token[MAX_TOKEN];
	int pos=0;
	double val=0.0;
	// ilk rakamın isareti
	c = geriVer( pv );
	if( c == '+' || c == '-' )
	token[pos++] = atla( pv );
	// noktadan sonra sını oku
	while( isdigit(geriVer(pv)) )
		token[pos++] = atla( pv );
	c = geriVer( pv );
	if( c == '.' )
		token[pos++] = atla( pv );
	// noktadan sonrasını oku
	while( isdigit(geriVer(pv)) )
		token[pos++] = atla( pv );
	c = geriVer( pv );
	while( isdigit(geriVer(pv) ) )
		token[pos++] = atla( pv );
		boslukAtla( pv );
    // string sonu kontrolu
   	token[pos] = '\0';
    // yanlıs karakter 
    if( pos == 0 || sscanf( token, "%lf", &val ) != 1 )
        ceviriHata( pv, "Failed to read real number" );
	return val;
}

/*parantez geldiğinde parantez içi komple yapılır ve değer tutulur*/
int argumanListOku( parsVeri *pv, int *num_args, double *args ){
	char c;

	// listeyi boşaltır
	*num_args = 0;

	// eboşlukları atlar
	boslukAtla( pv );
	while( geriVer( pv ) != ')' ){
		// arguman için verilen maximum size ı kontrol eder
		if( *num_args >= MAX_ARGUMENT )
		ceviriHata( pv, "Maximum arguman sayısına ulaşıldı !!!" );

		// elemanları okuyup listeye atarım
		args[*num_args] = islemOku( pv );
		*num_args = *num_args+1;
		boslukAtla( pv );

		// check the next character
		c = geriVer( pv );
		if( c == ')' ){
		// parantezler kapatılır
			break;
		} else {
			ceviriHata( pv, "parantezlerden biri açık kaldı!" );
			return 0;
		}
	}
	return 1;
}

double hepsiniYap( parsVeri *pv ){
	double sonuc1=0.0, sonuc2=0.0, args[MAX_ARGUMENT];
	char c, token[MAX_TOKEN];
	int num_args, pos=0;
	c = geriVer( pv );
	if( isalpha(c) ){
	while( isalpha(c) || isdigit(c)){
	    token[pos++] = atla( pv );
	    c = geriVer( pv );
	}
	token[pos] = '\0';
	// parantez açma varsa diye kontrol
	if( geriVer(pv) == '(' ){
	atla(pv);

	// özel fonksiyonlar kontrolü
	if( strcmp( token, "pow" ) == 0 ){
	    sonuc1 = argumanOku( pv );
	    sonuc2 = argumanOku( pv );
	    sonuc1 = pow( sonuc1, sonuc2 );
	} else if( strcmp( token, "sin" ) == 0 ){
	    sonuc1 = argumanOku( pv );    
	    sonuc1 = sin( sonuc1*PI/180 );
	} else if( strcmp( token, "cos" ) == 0 ){
	    sonuc1 = argumanOku( pv );
	    sonuc1 = cos( sonuc1*PI/180 );
	} else if( strcmp( token, "sqrt" ) == 0 ){
	    sonuc1 = argumanOku( pv );
	    if( sonuc1 < 0.0 )
	        ceviriHata( pv, "karekök içi negatif olamaz" );
	    sonuc1 = sqrt( sonuc1 );
	} else if( strcmp( token, "exp" ) == 0 ){
	    sonuc1 = argumanOku( pv );
	    sonuc1 = exp( sonuc1 );
	
	} else if( strcmp( token, "tan" ) == 0 ){
	    sonuc1 = argumanOku( pv );
	    sonuc1 = fmod(sonuc1,360);
	    if( sonuc1 == 90 || sonuc1 == 270 )
	        ceviriHata( pv, "tan x=90=270 için tanımsızdır!!!" );  
	    sonuc1 = tan( sonuc1*PI/180 );
	} else {
	    argumanListOku( pv, &num_args, args );
	        if( pv->fonksiyon && pv->fonksiyon( pv->myVeri, token, num_args, args, &sonuc2 ) ){
	            sonuc1 = sonuc2;
	        } else {
	            ceviriHata( pv, "Bilinmeyen özel terim var!" );
	        }
	}

	// parantez kapamalar
	if( atla( pv ) != ')' )
		ceviriHata( pv, "parantezlerden birisi açık kaldı!" );
	} else {
		// açık paranez kaldımı kontrolü
		if( pv->deger != NULL && pv->deger( pv->myVeri, token, &sonuc2 ) ){
			sonuc1 = sonuc2;
		} else {
			ceviriHata( pv, "Bilinmeyen hata oluştu" );
		}
	}
	} else {
	//özel fonkiyonlar geçildi double oku
	sonuc1 = doubleOku( pv );
	}
	boslukAtla( pv );
	return sonuc1;
}
/*özel fonksiyon okuma geldi sin,cos,exp,sqrt gibi*/
double argumanOku( parsVeri *pv ){
	char c;
	double val;
	boslukAtla( pv );

	// read the argument
	val = islemOku( pv );
	boslukAtla( pv );
	boslukAtla( pv );
	return val;
}
double parentezOku( parsVeri *pv ){
    double val;
    // parantez varmı
    if( geriVer( pv ) == '(' ){
	    atla( pv );
	    boslukAtla( pv );
	    val = parantezOkuyucu( pv );
	    boslukAtla( pv );
	    if( geriVer(pv) != ')' )
	    ceviriHata( pv, "Acık parentez var" );	
	    atla(pv);
    } else {
    	val = hepsiniYap( pv );
    }
    boslukAtla( pv );
    return val;
}

//üssü okuma için fonksiyon
double powOku( parsVeri *pv ){
    double sonuc1, sonuc2=1.0, s=1.0;
    sonuc1 = unaryOku( pv );//taban değer okunur
    boslukAtla( pv );
    while( geriVer(pv) == '^' ){
        atla(pv );
        boslukAtla( pv );
        if( geriVer( pv ) == '-' ){
        atla( pv );
        s = -1.0;
        boslukAtla( pv );
        }
        sonuc2 = s*powOku( pv );//üs okunur
        sonuc1 = pow( sonuc1, sonuc2 );
        boslukAtla( pv );
    }
    return sonuc1;
}
/*parante içini okur*/
double parantezOkuyucu( parsVeri *pv ){
	char c;
	double v;
	boslukAtla( pv );
	boslukAtla( pv );
	v = islemOku( pv );
	boslukAtla( pv );
	c = geriVer( pv );
	boslukAtla( pv );
	c = geriVer( pv );
	boslukAtla( pv );
	c = geriVer( pv );
	boslukAtla( pv );
	c = geriVer( pv );
	return v;
}
//binary işlemler okunur 
double ifadeOku( parsVeri *pv ){
    double sonuc1;
    char c;
    // ilk operand
    sonuc1 = powOku( pv );
    boslukAtla( pv );
    // sonraki karakter çarpma veya bölmemi
    c = geriVer( pv );
    while( c == '*' || c == '/' ){
        atla( pv );
        boslukAtla( pv );

        // işlemler yap
        if( c == '*' ){
        	sonuc1 *= powOku( pv );
        } else if( c == '/' ){
        	sonuc1 /= powOku( pv );
        }
        boslukAtla( pv );
        // karakteri değiştir
        c = geriVer( pv );
    }
return sonuc1;
}
/*toplama çıkarma işlemleri yapma*/
double islemOku( parsVeri *pv ){
    double sonuc1 = 0.0;
    char c;
    // unar çıkarma işlemi
    c = geriVer( pv );
    if( c == '+' || c == '-' ){
        atla( pv );
        boslukAtla( pv );
        if( c == '+' )
            sonuc1 += ifadeOku( pv );
        else if( c == '-' )
            sonuc1 -= ifadeOku( pv );
    } else {
        sonuc1 = ifadeOku( pv );
    }
    boslukAtla( pv );
    c = geriVer( pv );
    while( c == '+' || c == '-' ){
        atla( pv );
        boslukAtla( pv );
        if( c == '+' ){	
        sonuc1 += ifadeOku( pv );
        } else if( c == '-' ){
        sonuc1 -= ifadeOku( pv );
        }
        boslukAtla( pv );
        c = geriVer( pv );
    }
return sonuc1;
}
//negatif sayılar için okuma
double unaryOku( parsVeri *pv ){
    char c;
    double sonuc1;
    c = geriVer( pv );
    if( c == '!' ){
        ceviriHata( pv, "Yanlıs karakter girildi" );
    } else if( c == '-' ){
        atla(pv);
        boslukAtla(pv);
        sonuc1 = -parentezOku(pv);
    } else if( c == '+' ){
        atla( pv );
        boslukAtla(pv);
        sonuc1 = parentezOku(pv);
    } else {
        sonuc1 = parentezOku(pv);
    }
    boslukAtla(pv);
    return sonuc1;
}

/*--------END OF 111044070 localMServer.c------------*/
