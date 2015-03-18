
CC = gcc

CFLAGS = -g -O2 `pkg-config --cflags gtk+-2.0`

all: treeview-demo icon-with-text

treeview-demo: main.o
	gcc -o treeview-demo main.o `pkg-config --libs gtk+-2.0`

icon-with-text: main-icon-with-text.o
	gcc -o icon-with-text main-icon-with-text.o `pkg-config --libs gtk+-2.0`
clean:
	rm -f *.o treeview-demo icon-with-text 2>/dev/null
