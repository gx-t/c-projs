rel:
	gcc -Wall -s -O2 vt-test.c -l pthread -o vt-test
deb:
	gcc -Wall -g -DENABLE_TRACE vt-test.c -l pthread -o vt-test

tgz:
	tar czvf vt-test.tar.gz vt-test.c Makefile
