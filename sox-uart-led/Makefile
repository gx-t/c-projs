deb:
	gcc -Wall -g sox-uart-led.c -o sox-uart-led `pkg-config --libs sox`

rel:
	gcc -O2 -s -Wall -o sox-uart-led sox-uart-led.c `pkg-config --libs sox`

clean:
	rm sox-uart-led

