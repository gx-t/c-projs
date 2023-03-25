MAKEFLAGS+=-j 3

deb: 00-deb
rel: 00-rel

00-deb:
	gcc -Wall -g -shared -fPIC test.c -o test.so -lcrypto
00-rel:
	gcc -O3 -Wall -s -shared -fPIC test.c -o test.so -lcrypto

run: 00-rel
	python test.py

pyc:
	(echo import py_compile; echo 'py_compile.compile("test.py")') | python
	mv __pycache__/*.pyc .
	rm -rf __pycache__

clean:
	rm -rf *.so *.pyc __pycache__

