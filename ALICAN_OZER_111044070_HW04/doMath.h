#ifndef DO_MATH
#define DO_MATH
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef PI
#define PI 3.14159265
#endif

#define H_VALUE 0.0001//turev icin girilen h degeri
#define MAX_TOKEN 256//maximum string boyutu
#define MAX_ARGUMENT 10
#define CLIENTCOUNTER 20 //client sayisi
#define BILLION 1000000L //nanosecs to milisecs
#define MAX_BUF 1024
#define NUM_THREADS 30

FILE *filePtrLogFile;//fileptr
struct tm *timePtrTime;
char charPtrMyFifo[64];//serverin ana fifosu
int intClientCounter=0;//client counter
long int longIntDeltaTime;
int intPtrClients[CLIENTCOUNTER];
pthread_t threadPtrThreadIDs[NUM_THREADS];
char charPtrMyFunction[MAX_TOKEN];//SERVERA GELEN FONKSIYON

/*Main icindeki degiskenleri thread fonksiyonuna gondermek icin
structure olsturuyorum*/
struct myArgs{
	int intMyPid;//SERVERA BAGLANAN CLIENTIN PIDSI
	long int longIntClientTime;//CLIENTE GIRILEN SURE
};

typedef int (*callBackValue)( void *voidPtrValue, const char *charPtrName, double *doublePtrValue );
typedef int (*callBackFunction)( void *voidPtrValue, const char *charPtrName,
			 const int num_args, const double *args, double *doublePtrValue );

typedef struct {
	char charPtrExpression[256];//string**const char * idi***/
	int intLenght;//string uzunlugu
	int intPosition;//pozisyon
	jmp_buf	jmpBufferAdress;//bufferdaki eski konumu
	char charPtrErrorMessage[256];//charPtrErrorMessage
	void *voidPtrValue;//fonksiyondaki yerini gösterir
	callBackValue cbvParsingValue;//önceki okunan deger
	callBackFunction cbvParsingFunction;
} structParsData;

/*thread fonksiyonlri*/
void *turev(void *args);
void *integral(void *args);

/*signal handlers*/
void handlerSIGSET(int signo);
void handlerSIGPIPE(int signo);
/*herhangibir elemani atlamaizi saglar*/
char passArgument( structParsData *structGivenData );
/*üssü okuma için cbvParsingFunction*/
double readPower( structParsData *structGivenData );//
double readUnary( structParsData *structGivenData );
/*binary islemler okunur */
double readIfade( structParsData *structGivenData );
/*toplama cikarma islemleri yapma*/
double readOperator( structParsData *structGivenData );
/*stringdeki tüm bosluklari atlatir*/
void passWSpace( structParsData *structGivenData );
/*eger sayinin icinde nokta varsa noktayla beraber okur*/
double doubleOku( structParsData *structGivenData );
double baslaCevir( structParsData *structGivenData );
/*özel cbvParsingFunction okuma geldi sin,cos,exp,sqrt gibi*/
double readArgument( structParsData *structGivenData );
double hepsiniYap( structParsData *structGivenData );
/*parantez varsa okur icini yapar*/
double readBrace( structParsData *structGivenData );
/*okunann n inci karakteri döndürür*/
char geriVerN( structParsData *structGivenData, int n );
/*parantezleri okur ilem sonuclarini turar*/
double braceReader( structParsData *structGivenData );
/*parantez geldiginde parantez ici komple yapilir ve deger tutulur*/
int readArgumentList( structParsData *structGivenData, int *num_args, double *args);
/*çeviri durumda charPtrErrorMessage gelirse mesaj verir*/
void readingError( structParsData *structGivenData, const char *charPtrErr );
/*fonksiyonda  x ve w nun degerleri yerlestirilir*/
void stringCopier (char* charPtrTarget, char* charPtrGiven);
void stringReplacer(char first[],char x,char elem[]);
double cevirHep(const char *CharPtrExpression, callBackValue cbvParsingValue, callBackFunction cbvParsingFunction, void *voidPtrValue );
#endif