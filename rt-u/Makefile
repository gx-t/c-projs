deb:
	gcc -g -Wall -o rt-u rt-u.c

rel:
	gcc -O2 -s -Wall -o rt-u rt-u.c

deb32:
	gcc -g -m32 -Wall -o rt-u rt-u.c

rel32:
	gcc -O2 -m32 -s -Wall -o rt-u rt-u.c

arm:
	arm-linux-gnueabi-gcc -Wall -O2 -s -o rt-u rt-u.c

arm1:
	arm-unknown-linux-gnu-gcc -Wall -O2 -s -o rt-u rt-u.c

sfdf-test:
	./rt-u sfdf test/leve\&0-aaa test/\\evel0-bbb test/level0-cc\" | ./rt-u bin2txt > sfdf.log

sffd-test:
	./rt-u sffd test/leve\&0-aaa test/\\evel0-bbb test/level0-cc\" test/leve\`0-d\'d | ./rt-u bin2txt > sffd.log

sffd-test1:
	./rt-u sffd test/leve\&0-aaa test/leve\`0-d\'d | ./rt-u bin2txt > sffd.log

sfcl-test:
	./rt-u sfcl test | ./rt-u bin2txt level0-aaa.*ktt > sfcl.log

gds-test:
	./rt-u gds dump text < test/test_file.sf > gds.log

all-test:
	rm -f sfdf.log sffd.log sfcl.log
	make sfdf-test
	make sffd-test
	make sfcl-test
	make gds-test

