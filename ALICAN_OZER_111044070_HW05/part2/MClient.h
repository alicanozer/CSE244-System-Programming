#ifndef MCLIENT
#define MCLIENT
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

#ifndef PI
#define PI 3.14159265
#endif
#define MINPOINT 0
#define MAXPOINT 1
#define MAX_TOKEN 256//maximum string boyutu
#define MAX_ARGUMENT 10
#define SHMPERMS (IPC_CREAT | 0666)
#define SHMKEY 111044070
#define SEMNAME "/ozer"
#define MAXLEN 1024
#define MAX_THREAD 1000
#define TEN_BILLION 10000000L
#define FUNCTIONFILE "function.dat"
/**************************************/

/*shared memory structure*/
struct Data{
        double whellOfFunc[MAXLEN];
        int arrSize;
        int randNum;
        int pidClient; 
        int waitForServer;     
	};
/******************************/

struct Data *clientData;
sem_t *semClient;
int shmID=0;
pthread_t myThreads[MAX_THREAD];
int logFile;
int numOfThread=0;
int threadCounter=0;
char function[MAX_TOKEN];/*temp string for changing value*/
char turnWrapper[MAX_TOKEN];/*temp string*/

struct timespec sleeptime;

/***************************/
typedef int (*geriDeger)( void *myVeri, const char *name, double *value );
typedef int (*geriFonksiyon)( void *myVeri, const char *name,
			 const int num_args, const double *args, double *value );

/*parsing structure*/
typedef struct {
	const char *str;//string
	int len;//string uzunlugu
	int pos;//pozisyon
	jmp_buf	err_jmp_buf;//bufferdaki eski konumu
	const char *hata;//hata
	void *myVeri;//fonksiyondaki yerini gösterir
	geriDeger deger;//önceki okunan değer
	geriFonksiyon fonksiyon;
} parsVeri;

void SIGINTHandler(int signal);
void fileReader();
void *threadFunc(void *args);
/*herhangibir elemanı atlamaızı saglar*/
char atla( parsVeri *pd );
/*son okunan elemanı geri verir*/
char geriVer( parsVeri *pd );
/*üssü okuma için fonksiyon*/
double powOku( parsVeri *pd );//
double unaryOku( parsVeri *pd );
/*binary işlemler okunur */
double ifadeOku( parsVeri *pd );
/*toplama çıkarma işlemleri yapma*/
double islemOku( parsVeri *pd );
/*stringdeki tüm boşlukları atlatır*/
void boslukAtla( parsVeri *pd );
/*eger sayının içinde nokta varsa noktayla beraber okur*/
double doubleOku( parsVeri *pd );
double baslaCevir( parsVeri *pd );
/*özel fonksiyon okuma geldi sin,cos,exp,sqrt gibi*/
double argumanOku( parsVeri *pd );
double hepsiniYap( parsVeri *pd );
/*parantez varsa okur içini yapar*/
double parentezOku( parsVeri *pd );
/*pars işlemleri*/
double parantezOkuyucu( parsVeri *pd );
/*parantez geldiğinde parantez içi komple yapılır ve değer tutulur*/
int argumanListOku( parsVeri *pv, int *num_args, double *args);
/*çeviri durumda hata gelirse mesaj verir*/
void ceviriHata( parsVeri *pd, const char *err );
/*fonksiyonda  x ve w nun değerleri yerleştirilir*/
char *valueWrapper(const char *first, char pop, const char *push);
double cevirHep( const char *expr, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri );
int dataHazirla( parsVeri *pd, const char *str, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri );
#endif
