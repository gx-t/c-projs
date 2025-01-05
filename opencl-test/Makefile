deb: 00-deb

rel: 00-rel

00-deb:
	$(CC) -g -framework OpenCL -o 00-get-info 00-get-info.c
00-rel:
	$(CC) -O3 -framework OpenCL -o 00-get-info 00-get-info.c

clean:
	rm -rf 00-get-info

