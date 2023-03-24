MAKEFLAGS+=-j 3

deb: 00-deb
rel: 00-rel

00-deb:
	gcc -Wall -g -shared -fPIC test.c -o test.so -lcrypto
00-rel:
	gcc -O3 -Wall -s -shared -fPIC test.c -o test.so -lcrypto

run:
	python test.py

clean:
	rm *.so

