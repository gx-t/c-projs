deb:
	gcc -Wall -g *.c -o rt `pkg-config --cflags --libs gtk+-2.0`

rel:
	gcc -Wall -O2 -s *.c -o rt `pkg-config --cflags --libs gtk+-2.0`

clean:
	rm -rf rt

comp:
	tar -czvf rt.tar.gz *.c *.h Makefile treeview-tutorial

extr:
	tar -xvf rt.tar.gz

