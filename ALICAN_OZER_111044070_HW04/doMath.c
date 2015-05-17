/**----------doMath.c----------------
*--SYSTEM PROGRAMMING HW04-----------
*Author:Alican OZER------------------
*Date:29/04/2014  05:00 AM-----------
*--------MATHSERVER-PROGRAM----------
**/

#ifndef DO_MATH
#include "doMath.h"
#endif

void main(int argc,char **argv){
	signal(SIGINT, &handlerSIGSET);
	system("clear");
	if (argc != 3)
	{
		printf("HATALI PARAMETRE DiZiNi\n");
		printf("USAGE: %s \"function\" <time(nanosec)>\n", argv[0]);
		exit(0);
	}
	 
	long int longIntArg2 = atol(argv[2]);//nano
	if(longIntArg2 == 0)
	{
		printf("HATALI PARAMETRE DiZiNi\n");
		printf("USAGE: %s \"function\" <time(nanosec)>\n", argv[0]);
		exit(0);
	}
	strcpy(charPtrMyFunction,argv[1]);

	/*VARIALE-DEFINITION*/	
	char charPtrFifoBuffer[MAX_BUF];//sample buffer
	char charPtrLogFile[20];	
    int intFifoFileDesc;//ana fifo ddecriptor
    int intErrNo;
    long longThreadId=0;
    time_t currentTime;//zaman degiskenleri

    /*----------------*/
    longIntDeltaTime = longIntArg2/BILLION;    
    currentTime = time( NULL );
    timePtrTime = localtime( &currentTime );
    printf("Server[%d] Starting Time is %.2d:%.2d:%.2d waiting for client\n",
    			getpid(), timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec );
    /*--------------------------------*/   
    snprintf(charPtrMyFifo,64,"%d",getpid());
    mkfifo(charPtrMyFifo, 0666);//kendi pid sini fifo yapar serverpid
    intFifoFileDesc = open(charPtrMyFifo,O_RDONLY);    

    snprintf(charPtrLogFile,20,"SERVER%d.log",getpid());
	if((filePtrLogFile=fopen(charPtrLogFile,"a")) != NULL){
    	fprintf(filePtrLogFile,"[Server:%d] Starting Time is %.2d:%.2d:%.2d\n",
		getpid(),timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec );
    }
    else {printf("LOG FILE OLUSTURULAMADI\n");  exit(0);}				    


    while(1)
    {    	
    	signal(SIGINT, &handlerSIGSET);
    	signal(SIGPIPE,&handlerSIGPIPE);

        if(read(intFifoFileDesc,charPtrFifoBuffer,MAX_BUF) != 0)//client baglantisi
        {
        	intClientCounter++;       	
        	long int longIntClientTime; 				    
        	char pidTemp[10];//gecici pid stringi

			strncpy(pidTemp,charPtrFifoBuffer+1,10);//get pid from buf
			intPtrClients[intClientCounter-1]=atoi(pidTemp);
   				     	       	
	        	longIntClientTime=atoi(charPtrFifoBuffer+11);//milisaniye

	        	/*kapsulleme*/
	        	struct myArgs inMain;
	        		inMain.intMyPid=intPtrClients[intClientCounter-1];
					inMain.longIntClientTime=longIntClientTime;

		    	printf("Client [%c][%d] Connected:::%d\n",charPtrFifoBuffer[0],intPtrClients[intClientCounter-1],intClientCounter);    
		    	fprintf(filePtrLogFile,"Client [%charTemp][%d] Connected:::%d\n",charPtrFifoBuffer[0],intPtrClients[intClientCounter-1],intClientCounter);    		
            	if(charPtrFifoBuffer[0] == '1')
            	{

					intErrNo = pthread_create(threadPtrThreadIDs+longThreadId, NULL ,turev, (void *) &inMain);
					if (intErrNo) {
						printf("ERROR; return code from pthread_create() is %d\n", intErrNo);
						exit(-1);
					}

					pthread_detach(threadPtrThreadIDs[longThreadId]);						
					printf("Turev icin thread[%lu]::%ld\n", *(threadPtrThreadIDs+longThreadId),longThreadId);
					longThreadId++;
            	}
            	else if(charPtrFifoBuffer[0] == '2')
            	{
					intErrNo = pthread_create(threadPtrThreadIDs+longThreadId, NULL, integral, (void *) &inMain);
					if (intErrNo) {
						printf("ERROR; return code from pthread_create() is %d\n", intErrNo);
						exit(-1);
					}
					pthread_detach(threadPtrThreadIDs[longThreadId]);
					printf("Integral icin thread[%lu]::%ld\n", *(threadPtrThreadIDs+longThreadId),longThreadId);
					longThreadId++;            		
            	}
            	else
            	{
	            	printf("Unknown Client Connected!!!\n");
	            	kill(intPtrClients[intClientCounter-1],SIGINT);
	            }
        }		
    }
    printf("MAIN RETURNED!!!\n" );
}
void handlerSIGSET(int signo){

	time_t currentTime2;
    struct tm *myTime2;    
    currentTime2 = time( NULL );
    myTime2 = localtime( &currentTime2 );  
    int i;
    for (i = 0; i < intClientCounter; ++i)
    {
 		pthread_cancel(threadPtrThreadIDs[i]);
    	kill(intPtrClients[i],SIGINT);
    }
	printf("#####[PARENT].CTRL-C CAUGHT EXITING PROPERLY.[PARENT]#####\n");    
	if (intClientCounter > 0)
	{

		fprintf(filePtrLogFile,"Server CTRL-C catched children killed,and I died Time %.2d:%.2d:%.2d\n", 
			myTime2->tm_hour, myTime2->tm_min, myTime2->tm_sec );
		fclose(filePtrLogFile);
    }    
    unlink(charPtrMyFifo);  
	exit(1);
}
/*client kendini oldururse server mesaj verir*/
void handlerSIGPIPE(int signo){
	
	printf("Client Disconnected!!!\n");//consola bildirim 
	fprintf(filePtrLogFile,"Client Disconnected!!!\n");//consola bildirim
	pthread_exit(NULL);
}
/*ceviri durumda charPtrErrorMessage gelirse mesaj verir*/
void readingError( structParsData *structGivenData, const char *charPtrErr ){
    strcpy(structGivenData->charPtrErrorMessage,charPtrErr);
    longjmp( structGivenData->jmpBufferAdress, 1);
}
/*mainden structComes string icin struct olusturulur ve fonksiyonlar cagrilir*/
double cevirHep( const char *CharPtrExpression, callBackValue cbvParsingValue, callBackFunction cbvParsingFunction, void *voidPtrValue ){
    double doubleValue;
    structParsData structGivenData;

    strcpy(structGivenData.charPtrExpression,CharPtrExpression);
    structGivenData.cbvParsingFunction = cbvParsingFunction;
    structGivenData.voidPtrValue = voidPtrValue;
    strcpy(structGivenData.charPtrErrorMessage," ");    
    structGivenData.cbvParsingValue = cbvParsingValue;
    structGivenData.intPosition = 0;
    structGivenData.intLenght = strlen( CharPtrExpression )+1;

    doubleValue = baslaCevir( &structGivenData );
    if( strcmp(structGivenData.charPtrErrorMessage," ")){
        printf("Hata: %s\n", structGivenData.charPtrErrorMessage );
        printf("'%s' ceviri hatasi -nan- alindi\n", CharPtrExpression );
        printf("\n\tFONKSIYON DEGISKENINI x OLARAK GiRiNiZ==f(x)\n" );
        int i=0;
        for (i = 0; i < intClientCounter; ++i)
    	{
	 		pthread_cancel(threadPtrThreadIDs[i]);
	    	kill(intPtrClients[i],SIGINT);
    	}
        unlink(charPtrMyFifo);
        exit(0);
    }
    return doubleValue;
}
/*parsiing islemlerini baslatir */
double baslaCevir( structParsData *structGivenData ){
    double res = 0.0;
    if( !setjmp( structGivenData->jmpBufferAdress ) ){
        res = readOperator( structGivenData ); 
        passWSpace( structGivenData );
        if( structGivenData->intPosition < structGivenData->intLenght-1 ){
            readingError( structGivenData, "parsing yaparken charPtrErrorMessage olustu" );
        } else return res;
    } else {
        // charPtrErrorMessage was returned, output a nan silently
        return sqrt( -1.0 );
        }
    return sqrt(-1.0);
}
/*okunann n inci karakteri dondurur*/
char geriVerN( structParsData *structGivenData, int n ){
    if( structGivenData->intPosition+n < structGivenData->intLenght )
    	return structGivenData->charPtrExpression[structGivenData->intPosition+n];
    readingError( structGivenData, "Dizin okuma hatasi!!" );
    return '\0';
}
/*herhangi bir karakteri atlar*/
char passArgument( structParsData *structGivenData ){
    if( structGivenData->intPosition < structGivenData->intLenght )
    	return structGivenData->charPtrExpression[structGivenData->intPosition++];
    readingError( structGivenData, "Dizin okuma hatasi!" );
    return '\0';
}

/*stringdeki tum bosluklari atlatir*/
void passWSpace( structParsData *structGivenData ){
    while( isspace( geriVerN( structGivenData,0 ) ) )
    passArgument( structGivenData );
}
/*eger sayinin icinde nokta varsa noktayla beraber okur*/
double doubleOku( structParsData *structGivenData ){
	char charTemp, token[MAX_TOKEN];
	int intPosition=0;
	double doubleValue=0.0;
	// ilk rakamin isareti
	charTemp = geriVerN( structGivenData,0);
	if( charTemp == '+' || charTemp == '-' )
	token[intPosition++] = passArgument( structGivenData );
	// noktadan sonra sini oku
	while( isdigit(geriVerN(structGivenData,0)) )
		token[intPosition++] = passArgument( structGivenData );
	charTemp = geriVerN( structGivenData ,0);
	if( charTemp == '.' )
		token[intPosition++] = passArgument( structGivenData );
	// noktadan sonrasini oku
	while( isdigit(geriVerN(structGivenData,0)) )
		token[intPosition++] = passArgument( structGivenData );
	charTemp = geriVerN( structGivenData,0 );
	while( isdigit(geriVerN(structGivenData,0) ) )
		token[intPosition++] = passArgument( structGivenData );
		passWSpace( structGivenData );
    // string sonu kontrolu
   	token[intPosition] = '\0';
    // yanlis karakter 
    if( intPosition == 0 || sscanf( token, "%lf", &doubleValue ) != 1 )
        readingError( structGivenData, "Failed to read real number" );
	return doubleValue;
}

/*parantez geldiginde parantez ici komple yapilir ve deger tutulur*/
int readArgumentList( structParsData *structGivenData, int *intArgs, double *args ){
	char charTemp;

	// listeyi bosaltir
	*intArgs = 0;

	// ebosluklari atlar
	passWSpace( structGivenData );
	while( geriVerN( structGivenData ,0) != ')' ){
		// arguman icin verilen maximum size i kontrol eder
		if( *intArgs >= MAX_ARGUMENT )
		readingError( structGivenData, "Maximum arguman sayisina ulasildi !!!" );

		// elemanlari okuyup listeye atarim
		args[*intArgs] = readOperator( structGivenData );
		*intArgs = *intArgs+1;
		passWSpace( structGivenData );

		// check the next character
		charTemp = geriVerN( structGivenData ,0);
		if( charTemp == ')' ){
		// parantezler kapatilir
			break;
		} else {
			readingError( structGivenData, "parantezlerden biri acik kaldi!" );
			return 0;
		}
	}
	return 1;
}
//verilen charPtrGiven stringini charPtrTarget stringine kopyalar
void stringCopier (char* charPtrTarget, char* charPtrGiven) {
	int size=strlen(charPtrGiven);
	int i=0;
  	for (i=0; i<size; i++){
  		charPtrTarget[i] = charPtrGiven[i];
  	}
}
/*verilen charPtrGiven stringinde x elemanlarinin beldugu yere elem stringin koyar*/
void stringReplacer(char charPtrGiven[],char x,char elem[]){
	if(strchr(charPtrGiven,x) == NULL) return;
    int i,j;
    char ekli[256];
    memset(ekli,' ',256);        
    int size = strlen(elem);
    for (i = 0,j = 0; j < 256; ++i, j++)
    {
    	if ((charPtrGiven[i] == x) && (charPtrGiven[i+1] != 'p'))
    	{
    		stringCopier(ekli+j,elem);
    		j += size;    		
    	}
    	else
    	{
    	 	ekli[j]=charPtrGiven[i];
    	}    	
    }
    for (i=0,j=0; j < 256; ++j)
    {
    	if(!isspace(ekli[j])){
    		charPtrGiven[i]=ekli[j];
    		i++;
    	}
    }
    charPtrGiven[i]='\0';
}
double braceReader( structParsData *structGivenData ){
	char charTemp;
	double v;
	passWSpace( structGivenData );
	passWSpace( structGivenData );
	v = readOperator( structGivenData );
	passWSpace( structGivenData );
	charTemp = geriVerN( structGivenData ,0);
	passWSpace( structGivenData );
	charTemp = geriVerN( structGivenData ,0);
	passWSpace( structGivenData );
	charTemp = geriVerN( structGivenData ,0);
	passWSpace( structGivenData );
	charTemp = geriVerN( structGivenData ,0);
	return v;
}
/*stringten okunan argumanalrin eslerini bulur ve degistirir*/
double hepsiniYap( structParsData *structGivenData ){
	double doubleResult=0.0, sonuc2=0.0, args[MAX_ARGUMENT];
	char charTemp, token[MAX_TOKEN];
	int intArgs, intPosition=0;
	charTemp = geriVerN( structGivenData ,0);
	if( isalpha(charTemp) ){
	while( isalpha(charTemp) || isdigit(charTemp)){
	    token[intPosition++] = passArgument( structGivenData );
	    charTemp = geriVerN( structGivenData ,0);
	}
	token[intPosition] = '\0';
	// parantez acma varsa diye kontrol
	if( geriVerN(structGivenData,0) == '(' ){
	passArgument(structGivenData);

	// ozel fonksiyonlar kontrolu
	if( strcmp( token, "pow" ) == 0 ){
	    doubleResult = readArgument( structGivenData );
	    sonuc2 = readArgument( structGivenData );
	    doubleResult = pow( doubleResult, sonuc2 );
	} else if( strcmp( token, "sin" ) == 0 ){
	    doubleResult = readArgument( structGivenData );    
	    doubleResult = sin( doubleResult*PI/180 );
	} else if( strcmp( token, "cos" ) == 0 ){
	    doubleResult = readArgument( structGivenData );
	    doubleResult = cos( doubleResult*PI/180 );
	} else if( strcmp( token, "sqrt" ) == 0 ){
	    doubleResult = readArgument( structGivenData );
	    if( doubleResult < 0.0 )
	        readingError( structGivenData, "karekok ici negatif olamaz" );
	    doubleResult = sqrt( doubleResult );
	} else if( strcmp( token, "exp" ) == 0 ){
	    doubleResult = readArgument( structGivenData );
	    doubleResult = exp( doubleResult );
	
	} else if( strcmp( token, "tan" ) == 0 ){
	    doubleResult = readArgument( structGivenData );
	    doubleResult = fmod(doubleResult,360);
	    if( doubleResult == 90 || doubleResult == 270 )
	        readingError( structGivenData, "tan x=90=270 icin tanimsizdir!!!" );  
	    doubleResult = tan( doubleResult*PI/180 );
	} else {
	    readArgumentList( structGivenData, &intArgs, args );
	        if( structGivenData->cbvParsingFunction && structGivenData->cbvParsingFunction( structGivenData->voidPtrValue, token, intArgs, args, &sonuc2 ) ){
	            doubleResult = sonuc2;
	        } else {
	            readingError( structGivenData, "Bilinmeyen ozel terim var!" );
	        }
	}

	// parantez kapamalar
	if( passArgument( structGivenData ) != ')' )
		readingError( structGivenData, "parantezlerden birisi acik kaldi!" );
	} else {
		// acik paranez kaldimi kontrolu
		if( structGivenData->cbvParsingValue != NULL && structGivenData->cbvParsingValue( structGivenData->voidPtrValue, token, &sonuc2 ) ){
			doubleResult = sonuc2;
		} else {
			readingError( structGivenData, "Bilinmeyen charPtrErrorMessage olustu" );
		}
	}
	} else {
//ozel fonkiyonlar gecildi double oku
	doubleResult = doubleOku( structGivenData );
	}
	passWSpace( structGivenData );
	return doubleResult;
}
/*ozel cbvParsingFunction okuma geldi sin,cos,exp,sqrt gibi*/
double readArgument( structParsData *structGivenData ){
	char charTemp;
	double doubleValue;
	passWSpace( structGivenData );

	// read the argument
	doubleValue = readOperator( structGivenData );
	passWSpace( structGivenData );
	passWSpace( structGivenData );
	return doubleValue;
}
double readBrace( structParsData *structGivenData ){
    double doubleValue;
    // parantez varmi
    if( geriVerN( structGivenData ,0) == '(' ){
	    passArgument( structGivenData );
	    passWSpace( structGivenData );
	    doubleValue = braceReader( structGivenData );
	    passWSpace( structGivenData );
	    if( geriVerN(structGivenData,0) != ')' )
	    readingError( structGivenData, "Acik parentez var" );	
	    passArgument(structGivenData);
    } else {
    	doubleValue = hepsiniYap( structGivenData );
    }
    passWSpace( structGivenData );
    return doubleValue;
}

//ussu okuma icin cbvParsingFunction
double readPower( structParsData *structGivenData ){
    double doubleResult, sonuc2=1.0, s=1.0;
    doubleResult = readUnary( structGivenData );//taban deger okunur
    passWSpace( structGivenData );
    while( geriVerN(structGivenData,0) == '^' ){
        passArgument(structGivenData );
        passWSpace( structGivenData );
        if( geriVerN( structGivenData ,0) == '-' ){
        passArgument( structGivenData );
        s = -1.0;
        passWSpace( structGivenData );
        }
        sonuc2 = s*readPower( structGivenData );//us okunur
        doubleResult = pow( doubleResult, sonuc2 );
        passWSpace( structGivenData );
    }
    return doubleResult;
}
//binary islemler okunur 
double readIfade( structParsData *structGivenData ){
    double doubleResult;
    char charTemp;
    // ilk operand
    doubleResult = readPower( structGivenData );
    passWSpace( structGivenData );
    // sonraki karakter carpma veya bolmemi
    charTemp = geriVerN( structGivenData ,0);
    while( charTemp == '*' || charTemp == '/' ){
        passArgument( structGivenData );
        passWSpace( structGivenData );

        // islemler yap
        if( charTemp == '*' ){
        	doubleResult *= readPower( structGivenData );
        } else if( charTemp == '/' ){
        	doubleResult /= readPower( structGivenData );
        }
        passWSpace( structGivenData );
        // karakteri degistir
        charTemp = geriVerN( structGivenData ,0);
    }
return doubleResult;
}
/*toplama cikarma islemleri yapma*/
double readOperator( structParsData *structGivenData ){
    double doubleResult = 0.0;
    char charTemp;
    // unar cikarma islemi
    charTemp = geriVerN( structGivenData ,0);
    if( charTemp == '+' || charTemp == '-' ){
        passArgument( structGivenData );
        passWSpace( structGivenData );
        if( charTemp == '+' )
            doubleResult += readIfade( structGivenData );
        else if( charTemp == '-' )
            doubleResult -= readIfade( structGivenData );
    } else {
        doubleResult = readIfade( structGivenData );
    }
    passWSpace( structGivenData );
    charTemp = geriVerN( structGivenData ,0);
    while( charTemp == '+' || charTemp == '-' ){
        passArgument( structGivenData );
        passWSpace( structGivenData );
        if( charTemp == '+' ){	
        doubleResult += readIfade( structGivenData );
        } else if( charTemp == '-' ){
        doubleResult -= readIfade( structGivenData );
        }
        passWSpace( structGivenData );
        charTemp = geriVerN( structGivenData ,0);
    }
return doubleResult;
}
//negatif sayilar icin okuma
double readUnary( structParsData *structGivenData ){
    char charTemp;
    double doubleResult;
    charTemp = geriVerN( structGivenData ,0);
    if( charTemp == '!' ){
        readingError( structGivenData, "Yanlis karakter girildi" );
    } else if( charTemp == '-' ){
        passArgument(structGivenData);
        passWSpace(structGivenData);
        doubleResult = -readBrace(structGivenData);
    } else if( charTemp == '+' ){
        passArgument( structGivenData );
        passWSpace(structGivenData);
        doubleResult = readBrace(structGivenData);
    } else {
        doubleResult = readBrace(structGivenData);
    }
    passWSpace(structGivenData);
    return doubleResult;
}
/*turev fonksiyonu icin thread*/
void *turev(void *args){
	signal(SIGINT, &handlerSIGSET);
	char charPtrTurevFifo[20];//turevin ozel fifosu
	char charPtrSwapper1[256];//birinci islem icin exprs.
	char charPtrSwapper2[256];//ikinci islem icin exprs.
	char myX[20];//x degeri atmak icin
	char myXH[20];//x+h degerinin atamak icin
	char myW[20];//w degiskenini atamak icin
    long int longIntTotalTime=0;//toplam gecen sure
    long int longIntDifTime=0;//zaman farki
    double doubleResult1=0.0;//f(x)
    double doubleResult2=0.0;//F(x+h)fonksiyondan donen deger
    time_t timeLastTime;//cikis zamani
    struct timeval tvalBefore, tvalAfter;//zaman farki icin
    char charPtrFifoBuffer[MAX_BUF];//sample buffer
    int intFifoDescTurev;//turev fifo descriptoru      	

	struct myArgs *structComes = (struct myArgs *)args;
	const int intComesPID=structComes->intMyPid;
	const long int lifeTime=structComes->longIntClientTime;
	structComes=NULL;

	gettimeofday (&tvalBefore, NULL);
	snprintf(charPtrTurevFifo,20,"%d-DERIVATIVE",intComesPID);

	mkfifo(charPtrTurevFifo, 0666);//DERIVATIVE fifo
	intFifoDescTurev = open(charPtrTurevFifo,O_WRONLY);
	if (intFifoDescTurev < 0)
	{
		perror("FIFO HATASI:\n");
		pthread_exit(NULL);
	}
	for ( ; ; ) 
	{
		signal(SIGINT, &handlerSIGSET);
		usleep(longIntDeltaTime*1000);//miliseconds to microseconds
		longIntTotalTime += longIntDeltaTime;
		gettimeofday (&tvalAfter, NULL);
		longIntDifTime=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 + (tvalAfter.tv_usec - tvalBefore.tv_usec )/1000 );
		if (longIntDifTime > lifeTime)
		{
			time( &timeLastTime );
			timePtrTime = localtime( &timeLastTime );
			snprintf(charPtrFifoBuffer,MAX_BUF,"%d Client[D] Disconnected[Time:%.2d:%.2d:%.2d][ExecTime:%ld ms.]***\n",
			intComesPID,timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec,longIntDifTime);
    		write(intFifoDescTurev,charPtrFifoBuffer,MAX_BUF);
			snprintf(charPtrFifoBuffer,MAX_BUF,"FINISHED");//sonlandiricharTempi komutu
			write(intFifoDescTurev,charPtrFifoBuffer,MAX_BUF);
			printf("[%d]Client[D] Disconnected!!!\n", intComesPID);//consola bildirim 
			fprintf(filePtrLogFile,"[%d]Client[D] Disconnected!!!\n", intComesPID);//consola bildirim           			
			pthread_exit(NULL);
		}
		strcpy(charPtrSwapper1,charPtrMyFunction);//ana fonksiyona dokunmuyorum
		strcpy(charPtrSwapper2,charPtrMyFunction);//ana fonksiyona dokunmuyorum

/*expressin icinde 'w' varsa onunda degeri degistirilir*/
		if(strchr(charPtrSwapper1,'w') != NULL){
		    snprintf(myW,20,"%f",2*PI);//w ile degeri degistiri
		    stringReplacer(charPtrSwapper1,'w',myW);
		    stringReplacer(charPtrSwapper2,'w',myW);
		}
		/*f'(x) bulunur*/
	    snprintf(myX,20,"%f",(double)longIntDifTime);//x ile degistir
	    stringReplacer(charPtrSwapper1,'x',myX);
	    doubleResult1=cevirHep(charPtrSwapper1,NULL,NULL,NULL);
	    /*f'(x+h) bulunur*/
	    snprintf(myXH,20,"%f",(double)longIntDifTime+H_VALUE);//x+h ile degistir
	    stringReplacer(charPtrSwapper2,'x',myXH);
	    doubleResult2=cevirHep(charPtrSwapper2,NULL,NULL,NULL);
	    /*trapez kuralindan turev yapilir*/
	    doubleResult1 = (doubleResult2-doubleResult1)/H_VALUE;

			snprintf(charPtrFifoBuffer,MAX_BUF,"%d    %d %6ld   f'(%3ld)==%f\n",
										getpid(),intComesPID,longIntDifTime,longIntDifTime,doubleResult1);
	    if(write(intFifoDescTurev,charPtrFifoBuffer,MAX_BUF) != MAX_BUF){//sonuclar yolla cliente
	    	printf("[%d]Client[D] Disconnected!!!", intComesPID);//consola bildirim
	    	fprintf(filePtrLogFile,"[%d]Client[D] Disconnected!!!\n", intComesPID);//consola bildirim
	    	pthread_exit(NULL);
	    }		
	}    
}
/*integral icin thread fonksiyonu*/
void *integral(void *args){
	signal(SIGINT, &handlerSIGSET);
	char charPtrIntFifo[20];//integralin ozel fifosu
	char charPtrSwapper1[256];//birinci islem icin exprs.
	char charPtrSwapper2[256];//ikinci islem icin exprs.
	char myX[20];//x degerini atamak icin temp
	char myW[20];//w degerini atamak icin temp
    long int longIntTotalTime=0;//toplam gecen sure
    long int longIntDifTime=0;//zaman farki
    double doubleResult1=0.0;//f(x)
    double doubleResult2=0.0;//F(x+h)fonksiyondan donen deger
    time_t timeLastTime;//charTempikis zamani
    struct timeval tvalBefore, tvalAfter;//zaman farki icin
    char charPtrFifoBuffer[MAX_BUF];//sample buffer
    int intFifoDescInt;//int fifo descriptoru        	

	struct myArgs *structComes = (struct myArgs *)args;
	const int intComesPID=structComes->intMyPid;
	const long int lifeTime=structComes->longIntClientTime;
	structComes=NULL;

	gettimeofday (&tvalBefore, NULL);
	snprintf(charPtrIntFifo,20,"%d-INTEGRATE",intComesPID);

	mkfifo(charPtrIntFifo, 0666);//INTEGRATE fifo
	intFifoDescInt = open(charPtrIntFifo,O_WRONLY);
	if (intFifoDescInt < 0)
	{
		perror("FIFO HATASI:\n");
		pthread_exit(NULL);
	}
	for ( ; ; ) 
	{
		signal(SIGINT, &handlerSIGSET);
		usleep(longIntDeltaTime*1000);//miliseconds to microseconds
		longIntTotalTime += longIntDeltaTime;
		gettimeofday (&tvalAfter, NULL);
		longIntDifTime=((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000 + (tvalAfter.tv_usec - tvalBefore.tv_usec )/1000 );
		if (longIntDifTime > lifeTime)
		{
			time( &timeLastTime );
			timePtrTime = localtime( &timeLastTime );
			snprintf(charPtrFifoBuffer,MAX_BUF,"%d Client[I] Disconnected[Time:%.2d:%.2d:%.2d][ExecTime:%ld ms.]***\n",
			intComesPID,timePtrTime->tm_hour, timePtrTime->tm_min, timePtrTime->tm_sec,longIntDifTime);
    		write(intFifoDescInt,charPtrFifoBuffer,MAX_BUF);
			snprintf(charPtrFifoBuffer,MAX_BUF,"FINISHED");//sonlandiricharTempi komutu
			write(intFifoDescInt,charPtrFifoBuffer,MAX_BUF);
			printf("[%d]Client[I] Disconnected!!!\n", intComesPID);//consola bildirim 
			fprintf(filePtrLogFile,"[%d]Client[I] Disconnected!!!\n", intComesPID);//consola bildirim            			
			pthread_exit(NULL);
		}
		strcpy(charPtrSwapper1,charPtrMyFunction);
		strcpy(charPtrSwapper2,charPtrMyFunction);

		if(strchr(charPtrSwapper1,'w') != NULL){
		    snprintf(myW,20,"%f",2*PI);//w ile degeri degistiri
		    stringReplacer(charPtrSwapper1,'w',myW);
		    stringReplacer(charPtrSwapper2,'w',myW);
		}

	    snprintf(myX,20,"%ld",longIntDifTime);//x ile degistir
	    stringReplacer(charPtrSwapper1,'x',"0");
	    doubleResult1=cevirHep(charPtrSwapper1,NULL,NULL,NULL);

	    stringReplacer(charPtrSwapper2,'x',myX);
	    doubleResult2=cevirHep(charPtrSwapper2,NULL,NULL,NULL);
		doubleResult1 = (doubleResult2+doubleResult1)*(longIntDifTime)/2;
	    
			snprintf(charPtrFifoBuffer,MAX_BUF,"%d    %d %6ld   S(0:%3ld)==%f\n",
				getpid(),intComesPID,longIntDifTime,longIntDifTime,doubleResult1);
	    if(write(intFifoDescInt,charPtrFifoBuffer,MAX_BUF) != MAX_BUF){//sonuclar yolla cliente
	    	printf("[%d]Client[I] Disconnected!!!\n", intComesPID);//consola bildirim 
			fprintf(filePtrLogFile,"[%d]Client[I] Disconnected!!!\n", intComesPID);//consola bildirim 
			pthread_exit(NULL);
	    }	
	}       
}
/*--------END OF 111044070 SERVER.c------------*/