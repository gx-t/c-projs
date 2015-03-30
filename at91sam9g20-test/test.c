#include <stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
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



static int simple_blink_main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1) {
		perror("/dev/mem");
		return 2;
	}
	volatile void* map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xfffff000);
	close(fd);
	if(MAP_FAILED == map_base) {
		perror("mmap");
		return 3;
	}
	*(unsigned*)(map_base + 0x600) = 1; //enable port
	*(unsigned*)(map_base + 0x610) = 1; //enable output
	while(1) {
		*(unsigned*)(map_base + 0x630) = 1;
		usleep(100000);
		*(unsigned*)(map_base + 0x634) = 1;
		usleep(100000);
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

