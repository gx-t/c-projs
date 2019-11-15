deb: *.c
	gcc -Wall -g -o udp-test udp-test.c

rel: *.c
	gcc -Wall -O2 -s -o udp-test udp-test.c

deb.mips: *.c
	mipsel-openwrt-linux-gcc -Wall -g -o udp-test.mips udp-test.c

rel.mips: *.c
	mipsel-openwrt-linux-gcc -Wall -O2 -s -o udp-test.mips udp-test.c

tags:
	ctags -R . $(STAGING_DIR)/toolchain-mipsel_24kc_gcc-7.3.0_musl/include

clean:
	rm -f udp-test udp-test.mips tags

