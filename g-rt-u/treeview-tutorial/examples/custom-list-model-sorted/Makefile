
CC = gcc

OBJS = main.o custom-list.o

CFLAGS = -g -O2 `pkg-config --cflags gtk+-2.0`

customlist-sorted: $(OBJS)
	gcc -o customlist-sorted $(OBJS) `pkg-config --libs gtk+-2.0`

clean:
	rm $(OBJS) customlist-sorted 2>/dev/null || /bin/true 
