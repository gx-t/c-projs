MAKEFLAGS+=-j 4
MAC_PREFIX=-I`brew --prefix openssl`/include -L`brew --prefix openssl`/lib

.PHONY: none clean deb rel 00-deb-mac 00-rel-mac 00-deb 00-rel 01-deb 01-rel

none:
	@echo "run make <target>"

deb-mac: 00-deb-mac 01-deb
rel-mac: 00-rel-mac 01-rel

deb: 00-deb 01-deb
rel: 00-rel 01-rel

00-deb-mac:
	$(CC) -Wall -g -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto
00-rel-mac:
	$(CC) -O3 -Wall -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto

00-deb:
	$(CC) -Wall -g -Wno-deprecated-declarations udp-file.c -o udp-file -lcrypto
00-rel:
	$(CC) -O3 -s -Wall -s -Wno-deprecated-declarations udp-file.c -o udp-file -lcrypto

01-deb:
	$(CC) -Wall -g cam-ctrl.c -o cam-ctrl
01-rel:
	$(CC) -O3 -Wall cam-ctrl.c -o cam-ctrl

clean:
	rm -rf udp-file *.dSYM

