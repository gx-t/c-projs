#include <stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include "at91sam9g20.h"
//
//PIOB_PER Port Enable Register
//PIOB_OER Output Enable Register
//PIOB_SODR Set Output Data Register
//PIOB_CODR PIOB Clear Output Data Register
//PIOB_IFER Glitch filter enable register
//PIOB_PUDR Pull up disable register
//PIOB_PUER Pull up enable register
//PIOB_IDR Interrupt disable register
//http://forum.lazarus.freepascal.org/index.php?topic=21907.0
//https://www.fbi.h-da.de/fileadmin/personal/m.pester/mps/Termin2/Termin2.pdf
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

static int simple_blink_main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	volatile void* map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PIOB_BASE  & ~MAP_MASK);
	close(fd);
	while(1) {
		*((unsigned long *) (map_base + (PIOB_CODR & MAP_MASK))) = 1;
		usleep(200000);
		*((unsigned long *) (map_base + (PIOB_SODR & MAP_MASK))) = 1;
		usleep(200000);
	}
	munmap((void*)map_base, MAP_SIZE);
	return 0;
}

static int show_usage(int err) {
	const char* msg = "Usage: test <command> <args>\n"\
	"Commands:\n"\
	"\tsimple-blink";
	puts(msg);
	return err;
}

int main(int argc, char* argv[]) {
	if(argc < 2) return show_usage(1);
	argc --;
	argv ++;
	if(!strcmp(*argv, "simple-blink")) return simple_blink_main(argc, argv);
	fprintf(stderr, "Unknown command: %s\n", *argv);
	return show_usage(2);
}

