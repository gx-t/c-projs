
CC = gcc

OBJS = main.o cellrendererspin.o 

CFLAGS = -g -O2 `pkg-config --cflags gtk+-2.0`

spinbuttonrenderer: $(OBJS)
	gcc -o spinbuttonrenderer $(OBJS) `pkg-config --libs gtk+-2.0`

clean:
	rm $(OBJS) spinbuttonrenderer 2>/dev/null || /bin/true 
