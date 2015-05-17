all:
	clear	
	gcc -c doMath.c
	gcc -o doMath doMath.o -lm -pthread
	gcc -c turev.c
	gcc -o turev turev.o
	gcc -c integral.c
	gcc -o integral integral.o
	cat ./usage.txt
