/*----------------doMath.c-----------------------*/
/*--SYSTEM PROGRAMMING MIDTERM PROJECT-----------*/
/*Author:Alican OZER-----------------------------*/
/*Date:14/04/2014  04:00 AM----------------------*/
/*--------MATHSERVER-PROGRAM---------------------*/

#ifndef DO_MATH
#include "doMath.h"
#endif

void main(int argc,char **argv)
{
	signal(SIGINT, &sigHandler);
	system("clear");
	if (argc != 3)
	{
		printf("HATALI PARAMETRE DiZiNi\n");
		printf("USAGE: %s \"function\" <time(nanosec)>\n", argv[0]);
		exit(0);
	}
	long int nanoSeconds = atol(argv[2]);//nano 
	if(nanoSeconds == 0)
	{
		printf("HATALI PARAMETRE DiZiNi\n");
		printf("USAGE: %s \"function\" <time(nanosec)>\n", argv[0]);
		exit(0);
	}
	strcpy(function,argv[1]);
	long int miliSecondsServer = nanoSeconds/BILLION;
    pid_t pid;
    serverPID=(long)getpid();
    int fCounter=0;
    time_t currentTime;//zaman değişkenleri
    time_t lastTime;
    struct tm *myTime;
    currentTime = time( NULL );
    myTime = localtime( &currentTime );
    printf("Server[%ld] Starting Time is %.2d:%.2d:%.2d waiting for client\n",
    	(long)getpid(), myTime->tm_hour, myTime->tm_min, myTime->tm_sec );
    /*--------------------------------*/
    int fileDesc;//ana fifo ddecriptor
    
    long int miliSecondsClient;
    long int totalTime=0;
    
    snprintf(MYFIFO,64,"%ld",(long)getpid());
    mkfifo(MYFIFO, 0666);//kendi pid sini fifo yapar serverpid
    fileDesc = open(MYFIFO,O_RDONLY);
    long int value=0;
    double value2=0.0;//x+h
    double result=0.0;//f(x)
    double result2=0.0;//F(x+h)fonksiyondan dönen değer
    char functionID;//client belirteci
    if((logging=fopen(MYLOG,"a")) != NULL){
    	fprintf(logging,"[Server:%ld] Starting Time is %.2d:%.2d:%.2d\n",
    		(long)getpid(),myTime->tm_hour, myTime->tm_min, myTime->tm_sec );
    	fclose(logging);
    }
    else {printf("LOG FILE OLUSTURULAMADI\n");  exit(0);} 
    /*-dongu ctrlc yakalayna kadar döner client baglanırsa child olusur*/    
    while(1)
    {    	
    	signal(SIGINT, &sigHandler);    
        if(read(fileDesc,BUF,MAX_BUF) != 0)//client bağlantısı
        {
        	cCounter++;
            if ((pid = fork()) == -1)
            {            	
                perror("forking error:\n");
                exit(0);
            }          
            else if(pid == 0)/*CHILD PROCESS FUNCTIONS*/
            {    			
            	signal(SIGINT, &sigHandler);      	       	
	           	result=0.0;
	        	totalTime=0;
	        	functionID=BUF[0];//client hangisi
	        	miliSecondsClient=atol(BUF+11);//milisaniye
	        	
	        	strncpy(pidTemp,BUF+1,10);//get pid from buf
		    	printf("Client [%c][%ld] Connected:::%d\n",functionID,(long)getpid(),cCounter);        		
            	if (functionID == '1')
            	{
            		gettimeofday (&tvalBefore, NULL);
					snprintf(DERIVATIVE,20,"%ld-DERIVATIVE",atol(pidTemp));//cliente özel fifo
					mkfifo(DERIVATIVE, 0666);//int fifo
					logFileDescDer = open(DERIVATIVE,O_WRONLY);
					if (logFileDescDer < 0)
					{
						perror("FIFO HATASI:\n");
						exit(0);
					}
            		time( &currentTime );
            		myTime = localtime( &currentTime );   		
            		snprintf(BUF,MAX_BUF,"%ld %ld Client*D[%d] Connected   [Time:%.2d:%.2d:%.2d]***\n",
            		(long)getppid(),(long)getpid(),cCounter,myTime->tm_hour, myTime->tm_min, myTime->tm_sec);
            		write(logFileDescDer,BUF,MAX_BUF);
            		
            		for ( ; ; ) 
            		{
	         			usleep(miliSecondsServer*1000);//miliseconds to microseconds
            			totalTime += miliSecondsServer;
            			fCounter++;
            			gettimeofday (&tvalAfter, NULL);
            			value=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 +
            					 (tvalAfter.tv_usec - tvalBefore.tv_usec )/1000 );         			
	            		if ( value  > miliSecondsClient)
	            		{
	            			time( &lastTime );
	            			myTime = localtime( &lastTime );
    	snprintf(BUF,MAX_BUF,"%ld %ld Client*D[%d] Disconnected[Time:%.2d:%.2d:%.2d][ExecTime:%ld ms.]***\n",
    	(long)getppid(),(long)getpid(),cCounter,myTime->tm_hour, myTime->tm_min, myTime->tm_sec,value);
		            		write(logFileDescDer,BUF,MAX_BUF);
	            			snprintf(BUF,MAX_BUF,"FINISHED");//sonlandırıcı komutu
	            			write(logFileDescDer,BUF,MAX_BUF);//komut yolla
	            			printf("Client*D[%c][%ld] Disconnected:::%d\n",functionID,(long)getpid(), cCounter);//consola bildirim            			
	            			exit(1);
	            		}
					    /*--------------*/
					    snprintf(myW,20,"%f",2*PI);//w ile degeri degistiri
					    change1=valueWrapper(function,'w',myW);
					    value2=(double)value;
					    snprintf(myX,20,"%f",value2);//x ile degistir
					    change2=valueWrapper(change1,'x',myX);
					    result=cevirString(change2);
					    snprintf(myXH,20,"%f",value2+H_VALUE);//x+h ile degistir
					    change2=valueWrapper(change1,'x',myXH);
					    result2=cevirString(change2);
					    result = (result2-result)/H_VALUE;
					    //printf("[%d]Derivating[%d]\n",cCounter,fCounter);
	      				snprintf(BUF,MAX_BUF,"%ld %ld #Value:%8ld f'(%6ld):%f\n",
	      					(long)getppid(),(long)getpid(),value,value,result);
					    if(write(logFileDescDer,BUF,MAX_BUF) != MAX_BUF){//sonuclar yolla cliente
					    	printf("terminated\n");
					    	exit(0);
					    }           			            		
   					} 
				close(logFileDescDer);//turev fifo kapat
				unlink(DERIVATIVE);
            	}
            	else if(functionID == '2')
            	{
            		gettimeofday (&tvalBefore, NULL);
					snprintf(INTEGRATE,20,"%ld-INTEGRATE",atol(pidTemp));
					mkfifo(INTEGRATE, 0666);//İNTEGRAL fifo
					logFileDescInt = open(INTEGRATE,O_WRONLY);
					if (logFileDescInt < 0)
					{
						perror("FIFO HATASI:\n");
						exit(0);
					}
					time( &currentTime );
            		myTime = localtime( &currentTime );   		
            		snprintf(BUF,MAX_BUF,"%ld %ld Client*I[%d] Connected   [Time:%.2d:%.2d:%.2d]***\n",
            			(long)getppid(),(long)getpid(),cCounter,myTime->tm_hour, myTime->tm_min, myTime->tm_sec);
            		write(logFileDescInt,BUF,MAX_BUF);
            		for ( ; ; ) 
            		{
	         			usleep(miliSecondsServer*1000);//miliseconds to microseconds
            			totalTime += miliSecondsServer;
            			fCounter++;
            			gettimeofday (&tvalAfter, NULL);
            			value=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 + (tvalAfter.tv_usec - tvalBefore.tv_usec )/1000 );
	            		if (value > miliSecondsClient)
	            		{
	            			time( &lastTime );
	            			myTime = localtime( &lastTime );
		snprintf(BUF,MAX_BUF,"%ld %ld Client*I[%d] Disconnected[Time:%.2d:%.2d:%.2d][ExecTime:%ld ms.]***\n",
		(long)getppid(),(long)getpid(),cCounter,myTime->tm_hour, myTime->tm_min, myTime->tm_sec,value);
		            		write(logFileDescInt,BUF,MAX_BUF);
	            			snprintf(BUF,MAX_BUF,"FINISHED");//sonlandırıcı komutu
	            			write(logFileDescInt,BUF,MAX_BUF);
	            			printf("Client*I[%c][%ld] Disconnected:::%d\n",functionID, (long)getpid(),cCounter);//consola bildirim            			
	            			exit(1);
	            		}
					    /*-----------------*/
					    snprintf(myW,20,"%f",2*PI);//w ile degeri degistiri
					    change1=valueWrapper(function,'w',myW);
					    snprintf(myX,20,"%ld",value);//x ile degistir
					    change2=valueWrapper(change1,'x',"0");
					    result=cevirString(change2);
					    change2=valueWrapper(change1,'x',myX);
					    result2=cevirString(change2);
						result = (result2+result)*(value)/2;
					    /*-----------------------*/
	      				snprintf(BUF,MAX_BUF,"%ld %ld #Value:%8ld S(0,%6ld):%f\n",
	      					(long)getppid(),(long)getpid(),value,value,result);
					    if(write(logFileDescInt,BUF,MAX_BUF) != MAX_BUF){//sonuclar yolla cliente
					    	printf("terminated\n");
					    	exit(0);
					    }

	            		//printf("[%d]Integrating[%d]\n",cCounter,fCounter);	            		
   					} 
				close(logFileDescInt);
				unlink(INTEGRATE);      
            	}
            	else
            	{
	            	printf("Unknown Client Connected!!!\n");
	            	exit(0);
	            }
            }
            else if(pid > 0)
		    {
					/*parent do anything*/          
		    }
        }		
    }
}
void sigHandler(int signo){
	time_t currentTime;
    struct tm *myTime;    
    currentTime = time( NULL );
    myTime = localtime( &currentTime );

	if(serverPID == (long)getpid() ){//sinyal parenttan geldyse
		printf("\t\t CTRL-C CAUGHT EXITING PROPERLY...\n");
	    char BUF[MAX_BUF]="";
	    printf("Server CTRL-C catched children killed,and I died Time %.2d:%.2d:%.2d\n", 
	    			myTime->tm_hour, myTime->tm_min, myTime->tm_sec );
   		if (cCounter > 0)
   		{
		    close(logFileDescDer);
		    close(logFileDescInt);
	    }    	
	    unlink(MYFIFO);
	    free(turnWrapper);	
	}else
	{//sinyal childdan geldiyse
		printf("***[%ld] child killed\n",(long)getpid()); 	
	}
	kill(atol(pidTemp),SIGINT);
	exit(1);
}
/*çeviri durumda hata gelirse mesaj verir*/
void ceviriHata( parsVeri *pv, const char *err ){
    pv->hata = err;
    longjmp( pv->err_jmp_buf, 1);
}
double cevirString( const char *expr ){
return cevirHep( expr, NULL, NULL, NULL );
}
//fonksiyonda  x ve w nun değerleri yerleştirilir
char *valueWrapper(const char *first, char pop, const char *push) {
    int count = 0;
    const char *tempForWrapper;//stringin devamını tutmak için
    for(tempForWrapper=first; *tempForWrapper; tempForWrapper++)
        count += (*tempForWrapper == pop);
    size_t rlen = strlen(push);
    char *turnWrapper = malloc(strlen(first) + (rlen-1)*count + 1);
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
double cevirHep( const char *expr, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri ){
    double val;
    parsVeri pv;
    dataHazirla( &pv, expr, deger, fonksiyon, myVeri );
    val = baslaCevir( &pv );
    if( pv.hata ){
        printf("Hata: %s\n", pv.hata );
        printf("'%s' çeviri hatası -nan- alındı\n", expr );
        kill(getppid(),SIGINT);
        kill(atol(pidTemp),SIGINT);
        exit(0);
    }
    return val;	
}

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

void bosVer( parsVeri *pv ){
    free( pv );
}
/*parsiing baslıyor */
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
/*okunann n inci karakteri döndürür*/
char geriVerN( parsVeri *pv, int n ){
    if( pv->pos+n < pv->len )
    return pv->str[pv->pos+n];
    ceviriHata( pv, "Dizin okuma hatası!!" );
    return '\0';
}
/*herhangi bir karakteri atlar*/
char atla( parsVeri *pv ){
    if( pv->pos < pv->len )
    return pv->str[pv->pos++];
    ceviriHata( pv, "Dizin okuma hatası!" );
    return '\0';
}
parsVeri *yeniDataEkle( const char *str, geriDeger deger, geriFonksiyon fonksiyon, void *myVeri ){
    parsVeri *pv = malloc( sizeof( parsVeri ) );
    if( !pv ) return NULL;
    pv->fonksiyon = fonksiyon;
	pv->myVeri = myVeri;
	pv->hata = NULL;
    pv->str = str;    
    pv->pos = 0;  
    pv->deger = deger;
    pv->len = strlen( str )+1;    
    return pv;
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
/*--------END OF 111044070 SERVER.c------------*/