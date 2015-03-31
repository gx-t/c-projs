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
//http://www.keil.com/dd/docs/arm/atmel/sam9g20/at91sam9g20.h
//http://forum.arduino.cc/index.php?topic=258619.0
#define MAP_SIZE 4096UL
#define IOPB_PER(_b) (*(unsigned*)(_b + 0x600))
#define IOPB_OER(_b) (*(unsigned*)(_b + 0x610))
#define IOPB_SODR(_b) (*(unsigned*)(_b + 0x630))
#define IOPB_CODR(_b) (*(unsigned*)(_b + 0x634))


static int lib_piob_from_pin(int pin) {
	if(pin < 3 || pin == 17 || pin == 18 || pin > 31) return -1;
	return pin - 3;
}

static void* lib_open_base() {
	void* map_base = 0;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1) {
		perror("/dev/mem");
		return 0;
	}
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xfffff000);
	close(fd);
	if(MAP_FAILED == map_base) {
		perror("mmap");
		return 0;
	}
	return map_base;
}

static void lib_close_base(void* map_base) {
	munmap(map_base, MAP_SIZE);
}

static int simple_blink_main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	volatile void* map_base = lib_open_base();
	if(!map_base) return 3;
	IOPB_PER(map_base) = 1;
	IOPB_OER(map_base) = 1;
	while(1) {
		IOPB_SODR(map_base) = 1;
		usleep(100000);
		IOPB_CODR(map_base) = 1;
		usleep(100000);
	}
	lib_close_base((void*)map_base);
	return 0;
}

static int piob_onoff_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test piob-onoff on|off <pin>[<pin>...]\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int piob_onoff_main(int argc, char* argv[]) {
	if(argc < 2) return piob_onoff_show_usage(3, "Not enough arguments for piob-onoff");
	return 0;
}

static int show_usage(int err) {
	const char* msg = "Usage: test <command> <args>\n"\
	"Commands:\n"\
	"\tsimple-blink\n"\
	"\tpiob-onoff\n";
	fputs(msg, stderr);
	return err;
}

int main(int argc, char* argv[]) {
	if(argc < 2) return show_usage(1);
	argc --;
	argv ++;
	if(!strcmp(*argv, "simple-blink")) return simple_blink_main(argc, argv);
	if(!strcmp(*argv, "piob-onoff")) return piob_onoff_main(argc, argv);
	fprintf(stderr, "Unknown command: %s\n", *argv);
	return show_usage(2);
}

