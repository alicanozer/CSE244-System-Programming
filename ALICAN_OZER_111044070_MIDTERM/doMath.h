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
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef PI
#define PI 3.14159265
#endif

#define H_VALUE 0.0001//turev icin girilen h degeri
#define MAX_TOKEN 256//maximum string boyutu
#define MAX_ARGUMENT 10
#define CLIENTCOUNTER 100 //client sayısı
#define BILLION 1000000L //nanosecs to milisecs
#define MAX_BUF 1024
#define MYLOG "LogFileM.txt"

char DERIVATIVE[20];//turevin ozel fifosu
char INTEGRATE[20];//integralin ozel fifosu
char MYFIFO[64];//serverin ana fifosu
int logFileDescDer;//turev fifo descriptoru
int logFileDescInt;//int fifo descriptoru
long int serverPID;//clientların pid sini tutar
char BUF[MAX_BUF];//sample buffer
int cCounter=0;//client counter
char pidTemp[10];//geçici pid stringi
FILE *logging;//fileptr
struct timeval tvalBefore, tvalAfter;
char function[MAX_TOKEN];
char myX[20];
char myXH[20];
char myW[20];
char *change1;
char *change2;
char *turnWrapper;//stringin devamını tutmak için

typedef int (*geriDeger)( void *myVeri, const char *name, double *value );
typedef int (*geriFonksiyon)( void *myVeri, const char *name,
			 const int num_args, const double *args, double *value );

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

/*signal handler*/
void sigHandler(int signo);
/*herhangibir elemanı atlamaızı saglar*/
char atla( parsVeri *pd );
/*kullanılan alanları bosaltır*/
void bosVer( parsVeri *pd );
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
/*okunann n inci karakteri döndürür*/
char geriVerN( parsVeri *pv, int n );
/*pars işlemleri*/
double cevirString( const char *expr );
/*parantezleri okur ilem sonuclarını turar*/
double parantezOkuyucu( parsVeri *pd );
/*parantez geldiğinde parantez içi komple yapılır ve değer tutulur*/
int argumanListOku( parsVeri *pv, int *num_args, double *args);
/*çeviri durumda hata gelirse mesaj verir*/
void ceviriHata( parsVeri *pd, const char *err );
/*fonksiyonda  x ve w nun değerleri yerleştirilir*/
char *valueWrapper(const char *first, char pop, const char *push);
double cevirHep( const char *expr, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri );
parsVeri *yeniDataEkle( const char *str, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri );
int dataHazirla( parsVeri *pd, const char *str, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri );
#endif