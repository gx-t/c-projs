deb:
	arm-unknown-linux-gnu-gcc -Wall -g -o test test.c

rel:
	arm-unknown-linux-gnu-gcc -Wall -O2 -s -o test test.c

tag:
	ctags -R . ~/gcc-4.2.3-glibc-2.3.3/

graph:
	arm-unknown-linux-gnu-gcc -dr test.c
	egypt *.expand | dot -T pdf -o test.pdf
	rm -f a.out *.expand

clean:
	rm -f test test.pdf *.expand a.out

