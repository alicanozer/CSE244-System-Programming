all:
	clear
	gcc -c main.c -g
	gcc -o main main.o -pthread
	gcc -c mmm.c -g
	gcc -o mmm mmm.o -pthread
	gcc -c vm.c
	gcc -o vm vm.o -pthread
	cat usage.txt
	rm *.log