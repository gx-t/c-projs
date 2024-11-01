MAKEFLAGS+=-j 4
MAC_PREFIX=-I`brew --prefix openssl`/include -L`brew --prefix openssl`/lib \
-I`brew --prefix libjpeg`/include -L`brew --prefix libjpeg`/lib

.PHONY: none clean deb rel 00-deb-mac 00-rel-mac 00-deb 00-rel 01-deb 01-rel 02-deb-mac 02-rel-mac 02-deb 02-rel 

none:
	@echo "run make <target>"

deb-mac: 00-deb-mac 01-deb 02-deb-mac
rel-mac: 00-rel-mac 01-rel 02-rel-mac

deb: 00-deb 01-deb 02-deb
rel: 00-rel 01-rel 02-rel

00-deb-mac:
	$(CC) -Wall -g -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto
00-rel-mac:
	$(CC) -O3 -Wall -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto

00-deb:
	$(CC) -Wall -g -Wno-deprecated-declarations udp-file.c -o udp-file `pkg-config --cflags --libs libcrypto`
00-rel:
	$(CC) -O3 -s -Wall -s -Wno-deprecated-declarations udp-file.c -o udp-file `pkg-config --cflags --libs libcrypto`

01-deb:
	$(CC) -Wall -g cam-ctrl.c -o cam-ctrl
01-rel:
	$(CC) -O3 -Wall cam-ctrl.c -o cam-ctrl

02-deb-mac:
	$(CC) -Wall -g $(MAC_PREFIX) jpeg-ctrl.c -o jpeg-ctrl -ljpeg
02-rel-mac:
	$(CC) -O3 -Wall $(MAC_PREFIX) jpeg-ctrl.c -o jpeg-ctrl -ljpeg

02-deb:
	$(CC) -Wall -g jpeg-ctrl.c -o jpeg-ctrl `pkg-config --cflags --libs libjpeg`
02-rel:
	$(CC) -O3 -Wall jpeg-ctrl.c -o jpeg-ctrl -ljpeg `pkg-config --cflags --libs libjpeg`

clean:
	rm -rf udp-file cam-ctrl jpeg-ctrl *.dSYM

