.PHONY: deb rel

deb: *.c
	cc -Wall -g -o udp-test-1 udp-test-1.c

rel: *.c
	cc -Wall -O2 -s -o udp-test-1 udp-test-1.c

clean:
	rm -f udp-test-1

