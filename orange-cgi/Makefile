deb:
	gcc -g -Wall -o orange-cgi *.c

rel:
	gcc -O2 -s -Wall -o orange-cgi *.c
	cp orange-cgi $(HOME)/www/cgi-bin

clean:
	rm orange-cgi
