#COM
deb: comwrc_1.c
	gcc -g -o comwrc comwrc_1.c

rel: comwrc_1.c
	gcc -O2 -s -o comwrc comwrc_1.c

arm: comwrc_1.c
	arm-unknown-linux-gnu-gcc -O2 -s -o comwrc comwrc_1.c

arm1: comwrc_1.c
	arm-linux-gcc -O2 -s -o comwrc comwrc_1.c

#CGI
deb_cgi: com_cgi.c
	gcc -g -o com_cgi com_cgi.c

rel_cgi: *.c
	gcc -O2 -s -o com_cgi com_cgi.c

arm_cgi: *.c
	arm-unknown-linux-gnu-gcc -O2 -s -o com_cgi com_cgi.c

arm1_cgi: *.c
	arm-linux-gcc -O2 -s -o com_cgi com_cgi.c

#http server
deb_http: chttpd.c
	gcc -g -o chttpd chttpd.c

rel_http: chttpd.c
	gcc -O2 -s -o chttpd chttpd.c

arm_http: *.c
	arm-unknown-linux-gnu-gcc -O2 -s -o chttpd chttpd.c

arm1_http: *.c
	arm-linux-gcc -O2 -s -o chttpd chttpd.c

#httpcom
httpcom_deb:
	gcc -g -o httpcom httpcom.c

httpcom_rel:
	gcc -O2 -s -o httpcom httpcom.c

httpcom_arm:
	arm-linux-gcc -O2 -s -o httpcom httpcom.c

compress:
	tar -czvf httpcom.tar.gz httpcom.c Makefile html

extract:
	tar -xvf httpcom.tar.gz

