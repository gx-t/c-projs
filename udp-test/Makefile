deb: *.c
	gcc -g -o udp-test udp-test.c

rel: *.c
	gcc -O2 -s -o udp-test udp-test.c

arm: *.c
	arm-unknown-linux-gnu-gcc -O2 -s -o udp-test udp-test.c

clean:
	rm -f udp-test

