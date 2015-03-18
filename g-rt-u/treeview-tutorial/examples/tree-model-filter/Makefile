
CC = gcc

OBJS = guitreemodelfilter.o main-filter-rows.o 

CFLAGS = -g -O2 `pkg-config --cflags gtk+-2.0`

all: $(OBJS)
	gcc -o filter-rows $(OBJS) `pkg-config --libs gtk+-2.0`

clean:
	rm -f filter-rows $(OBJS) 2>/dev/null 
