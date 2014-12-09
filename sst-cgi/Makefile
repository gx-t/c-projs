deb:
	gcc -Wall -g sst-cgi.c -o sst.cgi -luuid
	cp sst.cgi www/cgi-bin
rel:
	gcc -Wall -O2 -s sst-cgi.c -o sst.cgi -luuid
	cp sst.cgi www/cgi-bin
clean:
	rm sst.cgi
	rm www/cgi-bin/sst.cgi
