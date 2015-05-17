all:
	clear
	gcc -c MServer.c
	gcc -c MClient.c -g
	gcc -o MServer MServer.o -pthread -lm
	gcc -o MClient MClient.o -pthread -lm
	rm *.log
