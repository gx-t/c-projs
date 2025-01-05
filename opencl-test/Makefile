MAKEFLAGS+=-j 2
.PHONY: deb rel 00-deb 00-rel 01-deb 01-rel clean

deb: 00-deb 01-deb

rel: 00-rel 01-deb

00-deb:
	$(CC) -Wall -g -framework OpenCL -o 00-get-info 00-get-info.c
00-rel:
	$(CC) -Wall -O3 -framework OpenCL -o 00-get-info 00-get-info.c

01-deb:
	$(CC) -Wall -g -framework OpenCL -o 01-simple-calc 01-simple-calc.c
01-rel:
	$(CC) -Wall -O3 -framework OpenCL -o 01-simple-calc 01-simple-calc.c

clean:
	rm -rf 00-get-info 01-simple-calc

