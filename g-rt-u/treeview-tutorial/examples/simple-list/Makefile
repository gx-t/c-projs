
CC = gcc

CFLAGS = -Wall -g -O2 `pkg-config --cflags gtk+-2.0`

all: simple-list

simple-list: list.o
	gcc -o simple-list list.o `pkg-config --libs gtk+-2.0`

clean:
	rm -f *.o simple-list *.c~ *.h~ 2>/dev/null
