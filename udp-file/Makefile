MAKEFLAGS+=-j 3
MAC_PREFIX=-I`brew --prefix openssl`/include -L`brew --prefix openssl`/lib

deb-mac:
	$(CC) -Wall -g -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto
rel-mac:
	$(CC) -O3 -Wall -Wno-deprecated-declarations $(MAC_PREFIX) udp-file.c -o udp-file -lcrypto

deb:
	$(CC) -Wall -g -Wno-deprecated-declarations udp-file.c -o udp-file -lcrypto
rel:
	$(CC) -O3 -s -Wall -s -Wno-deprecated-declarations udp-file.c -o udp-file -lcrypto

clean:
	rm -rf udp-file *.dSYM

