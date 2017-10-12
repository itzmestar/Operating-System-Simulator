
LIBS :=  

CFLAGS := -Wno-int-conversion -Wno-implicit-function-declaration

CC := gcc

all: oss child

oss: oss.o global.o
	$(CC) global.o oss.o -o oss

child: child.o global.o
	$(CC) global.o child.o -o child
	
global.o: global.c
 
clean:
	rm -f *.o *.log oss child