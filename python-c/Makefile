MAKEFLAGS+=-j 3
OPENSSL_PREFIX=$(shell brew --prefix openssl)
ifneq ($(OPENSSL_PREFIX)),)
    OPENSSL_PREFIX := -I$(OPENSSL_PREFIX)/include -L$(OPENSSL_PREFIX)/lib
endif

deb: 00-deb
rel: 00-rel

00-deb:
	$(CC) -Wall -g -shared $(OPENSSL_PREFIX) -fPIC test.c -o test.so -lcrypto
00-rel:
	$(CC) -O3 -Wall -shared $(OPENSSL_PREFIX) -fPIC test.c -o test.so -lcrypto

run: 00-rel
	python test.py

pyc:
	(echo import py_compile; echo 'py_compile.compile("test.py")') | python
	mv __pycache__/*.pyc .
	rm -rf __pycache__

clean:
	rm -rf *.so *.dSYM *.pyc __pycache__

