MAKEFLAGS+=-j 3
DEB=-Wall -g
REL=-Wall -O3 -s

deb: 00-deb
rel: 00-rel

00-deb:
	gcc -Wall -g -shared -fPIC test.c -o test.so -lcrypto
00-rel:
	gcc -Wall -s -shared -fPIC test.c -o test.so -lcrypto

run:
	python test.py

clean:
	rm *.so

