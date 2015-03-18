
CC = gcc

CFLAGS = -Wall -g -O2 `pkg-config --cflags gtk+-2.0`

PROG = hello-world

OBJS = $(PROG).o

all: $(OBJS)
	gcc -o $(PROG) $(OBJS) `pkg-config --libs gtk+-2.0`

clean:
	rm -f $(PROG) $(OBJS) *.c~ *.h~ 2>/dev/null
