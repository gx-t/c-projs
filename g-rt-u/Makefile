deb:
	gcc -Wall -g *.c -o gtk-rt `pkg-config --cflags --libs gtk+-2.0`

rel:
	gcc -Wall -O2 -s *.c -o gtk-rt `pkg-config --cflags --libs gtk+-2.0`

val:
	gcc -Wall -g *.c -o gtk-rt `pkg-config --cflags --libs gtk+-2.0`
	valgrind --leak-check=full --track-origins=yes ./gtk-rt 2> valgrind.log

clean:
	rm -rf gtk-rt

