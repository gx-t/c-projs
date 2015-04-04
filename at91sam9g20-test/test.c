#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
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

enum {
	ERR_OK = 0,
	ERR_ARGC,
	ERR_CMD,
	ERR_MMAP,
	ERR_PIN,
	ERR_VAL,
};

#define MAP_SIZE 4096UL
#define IOPB_BASE		0x600
#define IOPB_PER(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x00))
#define IOPB_OER(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x10))
#define IOPB_ODR(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x14))
#define IOPB_SODR(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x30))
#define IOPB_CODR(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x34))
#define IOPB_PDSR(_b)	(*(unsigned*)(_b + IOPB_BASE + 0x3C))

//board pin to bit shift for PIOB
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

static void lib_close_base(volatile void* map_base) {
	munmap((void*)map_base, MAP_SIZE);
}

static int simple_blink_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test simple-blink <pin>\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int simple_blink_main(int argc, char* argv[]) {
	if(argc != 2) return simple_blink_show_usage(ERR_ARGC, "Invalid number of arguments for simple-blink");
	argc --;
	argv ++;
	int port_bit = lib_piob_from_pin(atoi(*argv));
	if(port_bit == -1) return simple_blink_show_usage(ERR_PIN, "Invalid pin number. Only \"B\" pins are supported");
	volatile void* map_base = lib_open_base();
	if(!map_base) return ERR_MMAP;
	IOPB_PER(map_base) = 1 << port_bit;
	IOPB_OER(map_base) = 1 << port_bit;
	while(1) {
		IOPB_SODR(map_base) = 1 << port_bit;
		usleep(100000);
		IOPB_CODR(map_base) = 1 << port_bit;
		usleep(100000);
	}
	lib_close_base(map_base);
	return ERR_OK;
}

static int piob_onoff_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test piob-onoff on|off <pin>[<pin>...]\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int piob_onoff_state(int func, int argc, char* argv[]) {
	argc --;
	argv ++;
	unsigned flags = 0;
	while(argc --) {
		int port_bit = lib_piob_from_pin(atoi(*argv));
		if(port_bit == -1) {
			fprintf(stderr, "Invalid pin number: %s. Only \"B\" pins are supported\n", *argv);
			continue;
		}
		argv ++;
		flags |= (1 << port_bit);
	}
	if(!flags) return piob_onoff_show_usage(ERR_PIN, "No pins selected - nothing to do");
	volatile void* map_base = lib_open_base();
	if(!map_base) return ERR_MMAP;
	IOPB_PER(map_base) = flags;
	IOPB_OER(map_base) = flags;
	if(func) {
		IOPB_SODR(map_base) = flags;
	} else {
		IOPB_CODR(map_base) = flags;
	}
	lib_close_base(map_base);
	return ERR_OK;
}

static int piob_onoff_main(int argc, char* argv[]) {
	if(argc < 3) return piob_onoff_show_usage(ERR_ARGC, "Not enough arguments for piob-onoff");
	argc --;
	argv ++;
	if(!strcmp(*argv, "on")) return piob_onoff_state(1, argc, argv);
	if(!strcmp(*argv, "off")) return piob_onoff_state(0, argc, argv);
	return piob_onoff_show_usage(ERR_VAL, "Illegal state value for port");
}

static int piob_read_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test piob-read <pin>\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int piob_read_main(int argc, char* argv[]) {
	if(argc < 2) return piob_read_show_usage(ERR_ARGC, "Not enough arguments for piob-onoff");
	argc --;
	argv ++;
	int pin = atoi(*argv);
	int port_bit = lib_piob_from_pin(pin);
	if(port_bit == -1) return piob_read_show_usage(ERR_PIN, "Invalid pin number. Only \"B\" pins are supported");
	volatile void* map_base = lib_open_base();
	if(!map_base) return ERR_MMAP;
	IOPB_PER(map_base) = 1;
	IOPB_ODR(map_base) = 1 << port_bit;
	int data_bit = IOPB_PDSR(map_base) & (1 << port_bit);
	printf("pin %d %s\n", pin, data_bit ? "on":"off");
	lib_close_base(map_base);
	return ERR_OK;
}

static int show_usage(int err) {
	const char* msg = "Usage: test <command> <args>\n"\
	"Commands:\n"\
	"\tsimple-blink\n"\
	"\tpiob-onoff\n"\
	"\tpiob-read\n";
	fputs(msg, stderr);
	return err;
}

int main(int argc, char* argv[]) {
	if(argc < 2) return show_usage(ERR_ARGC);
	argc --;
	argv ++;
	if(!strcmp(*argv, "simple-blink")) return simple_blink_main(argc, argv);
	if(!strcmp(*argv, "piob-onoff")) return piob_onoff_main(argc, argv);
	if(!strcmp(*argv, "piob-read")) return piob_read_main(argc, argv);
	fprintf(stderr, "Unknown command: %s\n", *argv);
	return show_usage(ERR_CMD);
}

