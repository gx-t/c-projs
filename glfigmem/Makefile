deb:
	gcc -Wall -g -o glfigmem *.c -lGL -lGLU -lpng -lX11

rel:
	gcc -Wall -O2 -s -o glfigmem *.c -lGL -lGLU -lpng -lX11

deb32:
	gcc -Wall -g -m32 -o glfigmem *.c -lGL -lGLU -lpng -lX11

rel32:
	gcc -Wall -O2 -m32 -s -o glfigmem *.c -lGL -lGLU -lpng -lX11

clean:
	rm glfigmem

compress:
	tar czvf glfigmem.tar.gz *.c Makefile

extract:
	tar xvf glfigmem.tar.gz
