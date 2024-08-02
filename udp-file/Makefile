MAKEFLAGS+=-j 3
MAC_PREFIX=-I`brew --prefix openssl`/include -L`brew --prefix openssl`/lib

deb-mac:
	$(CC) -Wall -g $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto
rel-mac:
	$(CC) -O3 -Wall $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto

deb:
	$(CC) -Wall -g -shared udp-file.c -o udp-file -lcrypto
rel:
	$(CC) -O3 -s -Wall -s udp-file.c -o udp-file -lcrypto

clean:
	rm -rf udp-file *.dSYM

