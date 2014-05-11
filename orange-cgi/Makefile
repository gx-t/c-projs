deb:
	gcc -g -Wall -o orange-cgi *.c

rel:
	gcc -O2 -s -Wall -o orange-cgi *.c
	cp orange-cgi $(HOME)/www/cgi-bin

upload:
	scp orange-cgi.c shah32768@sdf.org:~/proj/orange-cgi
	scp ../../html/orange/index.html shah32768@sdf.org:~/html/orange

clean:
	rm orange-cgi
