MAKEFLAGS+=-j 3
OPENSSL_PREFIX=-I`brew --prefix openssl`/include -L`brew --prefix openssl`/lib

deb-mac: 00-deb-mac
rel-mac: 00-rel-mac

deb: 00-deb
rel: 00-rel

00-deb-mac:
	$(CC) -Wall -g -shared $(OPENSSL_PREFIX) -fPIC test.c -o test.so -lcrypto
00-rel-mac:
	$(CC) -O3 -Wall -shared $(OPENSSL_PREFIX) -fPIC test.c -o test.so -lcrypto

00-deb:
	$(CC) -Wall -g -shared -fPIC test.c -o test.so -lcrypto
00-rel:
	$(CC) -O3 -s -Wall -s -shared -fPIC test.c -o test.so -lcrypto

run-mac: 00-rel-mac
	python3 test.py

run: 00-rel
	python test.py

pyc:
	(echo import py_compile; echo 'py_compile.compile("test.py")') | python
	mv __pycache__/*.pyc .
	rm -rf __pycache__

clean:
	rm -rf *.so *.dSYM *.pyc __pycache__

