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

#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>

#include "localMServer.h"

int main(int argc, char const *argv[])
{	
    signal(SIGINT,sigHandler);	

    char log[30]; 
	int randNumLocal=0,bufSize=0,updateTime=0,shmID=0,diffTime=0,counter=0;
    struct timeval tvalStart,tvalBefore, tvalAfter;/*zaman farki icin*/
	struct Data* myData;
    time_t currentTime;/*zaman degiskenleri*/
    struct tm *timePtrTime;
    struct timespec sleeptime;
    int timeCounter=1;

    sleeptime.tv_sec=0;
    sleeptime.tv_nsec=TEN_BILLION;
    system("clear"); 
    if (argc != 4)
	{
		printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
		printf("USAGE>$ %s_<bufSize>_<numOfThread>_<updateTime>\n",argv[0] );
		exit(0);
	}
	
	bufSize=atoi(argv[1]);
    numOfThread=atoi(argv[2]);
    updateTime=atoi(argv[3]);

    if (bufSize < 1 || numOfThread < 1 || updateTime < 1)
    {
        printf("***///***USAGE ERROR PLEASE FOLLOW***///***\n");
        printf("USAGE>$ %s_<bufSize>_<numOfThread>_<updateTime>\n",argv[0] );
        printf("!!!PLEASE ENTER<POSITIVE>NUMBER AS ARGUMENTS!!!\n");
        exit(0);
    }
    snprintf(log,30,"MAIN-%d.log",getpid());
    mainLog=fopen(log,"w");
    printf("bufSize:%d\tnumOfThread:%d\tupdateTime:%d milisec.\n",
                                bufSize,numOfThread,updateTime );
    fprintf(mainLog,"bufSize:%d\tnumOfThread:%d\tupdateTime:%d milisec.\n"
                                ,bufSize,numOfThread,updateTime );
    
    /*foksiyonu dosyadan okur ve stringe atar*/
    fileReader();

    funcWhell=(double *)malloc(bufSize*sizeof(double));
    if(funcWhell == NULL ){
        perror("[main]Failed to allocate whellOfFuncion array");
        return 1;
    }

    threadID=(pthread_t *)malloc(numOfThread*sizeof(pthread_t));
    if(threadID == NULL){
        perror("[main]Failed to allocate threadID array");
        return 1;
    }
    srand(time(NULL));
    randNumLocal=rand()%bufSize;
    
    /*filling the created array*/
    for (counter = 0; counter < bufSize; ++counter){
    	funcWhell[counter]=(counter+1)*((double)(MAXPOINT-MINPOINT))/(bufSize+1);
    }
    /*creating shraed memory*/
	if((shmID = shmget((key_t)SHMKEY,sizeof(struct Data),SHMPERMS)) == -1){/*creat*/
		perror("[main]Failed to create shared memory segment");
		return 1;
	}
	/*attaching structure to shared memory*/
	if((myData = (struct Data*)shmat(shmID,NULL,0)) == (void*)-1){/*attack*/
		perror("[main]Failed to attach shared memory segment");
		if (shmctl(shmID, IPC_RMID, NULL) == -1)
			perror("[main-error]Failed to remove memory segment");
		return 1;
	}
	/*assigning values to shared memory segment*/
	myData->whellOfFunc=funcWhell;
	myData->arrSize=bufSize;
	myData->randNum=randNumLocal;
    myData->status=1;
/*------------------------------*/    

    /* initialize the semaphore */
    if(sem_init(&sema, 0, 0) == -1){
    	perror("Failed to initialize semaphore!!!");
    	return 1;
    }

    /* create and detach several threads */
    for (counter = 0; counter < numOfThread; ++counter){
        pthread_create(threadID+counter, NULL, ThreadFunc, NULL);
        pthread_detach(threadID[counter]);
    }
    
    gettimeofday (&tvalBefore, NULL);/*for time different of averaging*/
    gettimeofday (&tvalStart, NULL);/*for keeping starting time*/
     
    currentTime = time( NULL );
    timePtrTime = localtime( &currentTime );
    /*----------------------------*/
    fprintf(mainLog,"localMServer[Main] Started %.2d:%.2d:%.2d\n",
        timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec );
    fprintf(mainLog,"----------------------------------------\n");
    fprintf(mainLog,"-AVERAGE--&--TIME---\n");

    printf("RUNNING.....$\n");
    /* run each thread until catch ctrl-c */
    while(1){        
        gettimeofday(&tvalAfter,NULL);
        diffTime=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 + 
        (tvalAfter.tv_usec - tvalBefore.tv_usec )/1000 );
        if(diffTime >= updateTime){
            
	        averaging(myData->whellOfFunc,myData->arrSize,updateTime*timeCounter
	                                    ,&(myData->status));
	        srand(time(NULL));
	        myData->randNum=rand()%bufSize;
	        gettimeofday (&tvalBefore, NULL);
	        diffTime=0;
	        timeCounter++;

    	}
        /* release a thread to execute */
    	sem_post(&sema);
    	nanosleep(&sleeptime,NULL);
    }
    return 0;
}
/*
**THREAD FUNCTION
*/
void *ThreadFunc(void *args)
{
    int thrID=0,myLog;
    struct Data *thrData;
    char BUF[1024];
    struct tm *timePtrTime;   
    struct timeval started,finished;/*zaman farki icin*/
    gettimeofday (&started, NULL);
    double value=0.0;    
    time_t currentTime;/*zaman degiskenleri*/
    char logFile[30];
    char assign[MAX_TOKEN];
    char myX[20];

    /*clearing segments of string*/
    memset(myX,0,20*sizeof(char));
    memset(assign,0,MAX_TOKEN*sizeof(char));

    /*connecting shared memory*/
    if((thrID = shmget((key_t)SHMKEY,sizeof(struct Data),SHMPERMS)) == -1){/*creat*/
        perror("[thread]Failed to create shared memory segment");
        return;
    }
    /*attacing data to shared memory to use it*/
    if((thrData = (struct Data*)shmat(thrID,NULL,0)) == (void*)-1){/*attack*/
        perror("[thread]Failed to attach shared memory segment");
        if (shmctl(thrID, IPC_RMID, NULL) == -1)
            perror("[thread-error]Failed to remove memory segment");
        return;
    }
    /*-------------*/   
    snprintf(logFile,30,"THREAD-%ld.log",(long int)pthread_self());
    if((myLog=open(logFile,O_WRONLY | O_APPEND | O_CREAT,S_IRUSR | S_IWUSR)) == -1){
    	perror("file open error");
    	return;
    }
    /*starting time */
    currentTime = time( NULL );
    timePtrTime = localtime( &currentTime );
    /*--------------------*/
    
    sprintf(BUF,":::::::::THREAD-%ld.log:::::::\n",(long int)pthread_self());
    write(myLog,BUF ,strlen(BUF));

    sprintf(BUF,"localMServer[Thread] Started %.2d:%.2d:%.2d\n",
        timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec);
    write(myLog,BUF ,strlen(BUF));

    sprintf(BUF,"-------------------------------------\n");
    write(myLog, BUF,strlen(BUF));

    sprintf(BUF,"---f(x)---&----(x)----&--(Time)------\n");
    write(myLog, BUF,strlen(BUF));

   	while (1)
    {
        /* wait for semaphore to be signalled */
        sem_wait(&sema);
        /*if server calclating average status is 0 else 1*/
        /*waiting thread when server calculating*/
        if(thrData->status == 1){
            if (thrData->randNum >= thrData->arrSize)
            	thrData->randNum=0;

            /*clearing datas*/
            memset(turnWrapper,0,MAX_TOKEN*sizeof(char));
            memset(assign,0,MAX_TOKEN*sizeof(char));
            memset(myX,0,20*sizeof(char));

            /*calculating value*/
            value=thrData->whellOfFunc[thrData->randNum];
            snprintf(myX,20,"%f",value);//x ile degistir
			strcpy(assign,valueWrapper(function,'x',myX));
			thrData->whellOfFunc[thrData->randNum]=cevirHep(assign,NULL,NULL,NULL);

			/*difference time*/
            gettimeofday (&finished, NULL);
            sprintf(BUF,"%f \t %f \t %6ld\n",thrData->whellOfFunc[thrData->randNum],value,
        	(finished.tv_sec - started.tv_sec)*1000 + (finished.tv_usec - started.tv_usec )/1000);
            write(myLog, BUF,strlen(BUF));
            thrData->randNum++;
        }
        else 
        	printf("MAIN GETTING AVERAGE--\n");     
    }
    return NULL;

}
/*
*SIGNAL HANDLER
*/
void sigHandler(int signo){
	int sigID,counter;

    printf(":::SERVER ctrl-c CATCHED:::\n");    
    fprintf(mainLog,":::SERVER ctrl-c CATCHED:::\n");

    fclose(mainLog);

    /*cancelling thread for clearly exit*/
	for (counter = 0; counter < numOfThread; ++counter)
    {
        pthread_cancel(threadID[counter]);
    }
    /*getting shared memory id for remove it*/
    if((sigID = shmget((key_t)SHMKEY,sizeof(struct Data),SHMPERMS)) == -1){
        perror("[signal]Failed to create shared memory segment");
        return;
    }

	/*destroying created semaphore*/
	if(sem_destroy(&sema) == -1)
		perror("[signal]Failed to destroy semaphore");    

	/*remove it from memory*/
	if (shmctl(sigID, IPC_RMID, NULL) == -1)
       perror("[signal]Failed to remove memory segment");    

   	free(threadID);
    free(funcWhell);
    sleep(1);
	exit(signo);
}
/*
*THIS FUNCTION CALCULATE AVERAGE OF ARRAY
*/
void averaging(double* whell,int size,long int times,int *st)
{
	*st=0;/*stopping threads for changing value*/
    double sum=0.0;
    int counter=0;

   	for (counter = 0; counter < size; ++counter)
      	sum += whell[counter]; 

//	printf("ORT:%f\n",sum/size);
   	fprintf(mainLog,"%f\t%ld\n",sum/size,times);
   	*st=1;/*start threads*/
}
/*
*READ EXPRESSINO FROM FILE AND ASSIGN IT TO STRNG
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

    if(function == NULL ){
        perror("[fileReader]Failed to allocate function array");
        exit(0);
    }    
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
