#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


enum {
	ERR_OK = 0,
	ERR_ARGC,
	ERR_CMD,
	ERR_MMAP,
	ERR_PIN,
	ERR_VAL,
};



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
//https://code.google.com/p/embox/source/browse/trunk/embox/src/include/drivers/at91sam7_tcc.h?spec=svn2952&r=2952

typedef volatile unsigned at91_reg_t;

//*****************************************************************************
//** GPIO
//*****************************************************************************
#define PIO_BASE		0xfffff000 //Input Output base address
#define PIO_B(_b)		((struct AT91S_PIO*)(_b + 0x600))

struct AT91S_PIO {
	at91_reg_t PIO_PER;       // PIO Enable Register
	at91_reg_t PIO_PDR;       // PIO Disable Register
	at91_reg_t PIO_PSR;       // PIO Status Register
	at91_reg_t Reserved0[1];  //
	at91_reg_t PIO_OER;       // Output Enable Register
	at91_reg_t PIO_ODR;       // Output Disable Registerr
	at91_reg_t PIO_OSR;       // Output Status Register
	at91_reg_t Reserved1[1];  //
	at91_reg_t PIO_IFER;      // Input Filter Enable Register
	at91_reg_t PIO_IFDR;      // Input Filter Disable Register
	at91_reg_t PIO_IFSR;      // Input Filter Status Register
	at91_reg_t Reserved2[1];  //
	at91_reg_t PIO_SODR;      // Set Output Data Register
	at91_reg_t PIO_CODR;      // Clear Output Data Register
	at91_reg_t PIO_ODSR;      // Output Data Status Register
	at91_reg_t PIO_PDSR;      // Pin Data Status Register
	at91_reg_t PIO_IER;       // Interrupt Enable Register
	at91_reg_t PIO_IDR;       // Interrupt Disable Register
	at91_reg_t PIO_IMR;       // Interrupt Mask Register
	at91_reg_t PIO_ISR;       // Interrupt Status Register
	at91_reg_t PIO_MDER;      // Multi-driver Enable Register
	at91_reg_t PIO_MDDR;      // Multi-driver Disable Register
	at91_reg_t PIO_MDSR;      // Multi-driver Status Register
	at91_reg_t Reserved3[1];  //
	at91_reg_t PIO_PPUDR;     // Pull-up Disable Register
	at91_reg_t PIO_PPUER;     // Pull-up Enable Register
	at91_reg_t PIO_PPUSR;     // Pull-up Status Register
	at91_reg_t Reserved4[1];  //
	at91_reg_t PIO_ASR;       // Select A Register
	at91_reg_t PIO_BSR;       // Select B Register
	at91_reg_t PIO_ABSR;      // AB Select Status Register
	at91_reg_t Reserved5[9];  //
	at91_reg_t PIO_OWER;      // Output Write Enable Register
	at91_reg_t PIO_OWDR;      // Output Write Disable Register
	at91_reg_t PIO_OWSR;      // Output Write Status Register
};

#define MAP_SIZE		4096UL

//*****************************************************************************
//** Timer Counter
//*****************************************************************************
#define TCB_BASE		0xFFFA0000 //Timer Counter Block base address

struct AT91S_TC {
	at91_reg_t		TC_CCR;		// Channel Control Register
	at91_reg_t		TC_CMR;		// Channel Mode Register (Capture Mode / Waveform Mode)
	at91_reg_t		Reserved0[2];//
	at91_reg_t		TC_CV;		// Counter Value
	at91_reg_t		TC_RA;		// Register A
	at91_reg_t		TC_RB;		// Register B
	at91_reg_t		TC_RC;		// Register C
	at91_reg_t		TC_SR;		// Status Register
	at91_reg_t		TC_IER;		// Interrupt Enable Register
	at91_reg_t		TC_IDR;		// Interrupt Disable Register
	at91_reg_t		TC_IMR;		// Interrupt Mask Register
};


// -------- TC_CCR : (TC Offset: 0x0) TC Channel Control Register --------
#define TC_CLKEN					(0x1 << 0)		//TC Clock Enable bit
#define TC_CLKDIS					(0x1 << 1)		//TC Clock Disable bit
#define TC_SWTRG					(0x1 << 2)		//TC Software Trigger

// -------- TC_CMR : (TC Offset: 0x4) TC Channel Mode Register: Capture Mode / Waveform Mode --------
#define TC_CLKS_XC0					0x5				//TC Clock Select: XC0
#define TC_CLKS_XC1					0x6				//TC Clock Select: XC1
#define TC_CLKS_XC2					0x7				//TC Clock Select: XC2
#define TC_ETRGEDG					(0x3 << 8)		//TC External Trigger Edge Selection
#define TC_ETRGEDG_NONE				(0x0 << 8)		//TC Edge: None
#define TC_ETRGEDG_RISING			(0x1 << 8)		//TC Edge: Rising
#define TC_ETRGEDG_FALLING			(0x2 << 8)		//TC Edge: Falling
#define TC_ETRGEDG_BOTH				(0x3 << 8)		//TC Edge: Both

struct AT91S_TCB {
	struct AT91S_TC		TCB_TC0;       // TC Channel 0
	at91_reg_t			Reserved0[4];  //
	struct AT91S_TC		TCB_TC1;       // TC Channel 1
	at91_reg_t			Reserved1[4];  //
	struct AT91S_TC		TCB_TC2;       // TC Channel 2
	at91_reg_t			Reserved2[4];  //
	at91_reg_t			TCB_BCR;       // TC Block Control Register
	at91_reg_t			TCB_BMR;       // TC Block Mode Register
};

// -------- TCB_BCR : (TCB Offset: 0xc0) TC Block Control Register --------
#define AT91C_TCB_SYNC				((unsigned int) 0x1 <<  0)	// (TCB) Synchro Command
// -------- TCB_BMR : (TCB Offset: 0xc4) TC Block Mode Register --------
#define AT91C_TCB_TC0XC0S			((unsigned int) 0x3 <<  0)	// (TCB) External Clock Signal 0 Selection
#define	AT91C_TCB_TC0XC0S_TCLK0		((unsigned int) 0x0 <<  0)	// (TCB) TCLK0 connected to XC0
#define	AT91C_TCB_TC0XC0S_NONE		((unsigned int) 0x1 <<  0)	// (TCB) None signal connected to XC0
#define AT91C_TCB_TC0XC0S_TIOA1		((unsigned int) 0x2 <<  0)	// (TCB) TIOA1 connected to XC0
#define AT91C_TCB_TC0XC0S_TIOA2		((unsigned int) 0x3 <<  0)	// (TCB) TIOA2 connected to XC0
#define AT91C_TCB_TC1XC1S			((unsigned int) 0x3 <<  2)	// (TCB) External Clock Signal 1 Selection
#define AT91C_TCB_TC1XC1S_TCLK1		((unsigned int) 0x0 <<  2)	// (TCB) TCLK1 connected to XC1
#define AT91C_TCB_TC1XC1S_NONE		((unsigned int) 0x1 <<  2)	// (TCB) None signal connected to XC1
#define AT91C_TCB_TC1XC1S_TIOA0		((unsigned int) 0x2 <<  2)	// (TCB) TIOA0 connected to XC1
#define AT91C_TCB_TC1XC1S_TIOA2		((unsigned int) 0x3 <<  2)	// (TCB) TIOA2 connected to XC1
#define AT91C_TCB_TC2XC2S			((unsigned int) 0x3 <<  4)	// (TCB) External Clock Signal 2 Selection
#define AT91C_TCB_TC2XC2S_TCLK2		((unsigned int) 0x0 <<  4)	// (TCB) TCLK2 connected to XC2
#define AT91C_TCB_TC2XC2S_NONE		((unsigned int) 0x1 <<  4)	// (TCB) None signal connected to XC2
#define AT91C_TCB_TC2XC2S_TIOA0		((unsigned int) 0x2 <<  4)	// (TCB) TIOA0 connected to XC2
#define AT91C_TCB_TC2XC2S_TIOA1		((unsigned int) 0x3 <<  4)	// (TCB) TIOA2 connected to XC2


//board pin to bit shift for PIOB
static int lib_piob_from_pin(int pin) {
	if(pin < 3 || pin == 17 || pin == 18 || pin > 31) return -1;
	return pin - 3;
}

static void* lib_open_base(off_t offset) {
	void* map_base = 0;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1) {
		perror("/dev/mem");
		return 0;
	}
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
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
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	volatile struct AT91S_PIO* piob = PIO_B(map_base);
	unsigned flags = 1 << port_bit;
	piob->PIO_PER = flags;
	piob->PIO_OER = flags;
	while(1) {
		piob->PIO_SODR = flags;
		usleep(100000);
		piob->PIO_CODR = flags;
		usleep(100000);
	}
	lib_close_base(map_base);
	return ERR_OK;
}

static int piob_write_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test piob-write on|off <pin>[<pin>...]\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int piob_write_state(int func, int argc, char* argv[]) {
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
	if(!flags) return piob_write_show_usage(ERR_PIN, "No pins selected - nothing to do");
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	volatile struct AT91S_PIO* piob = PIO_B(map_base);
	piob->PIO_PER = flags;
	piob->PIO_OER = flags;
	if(func) {
		piob->PIO_SODR = flags;
	} else {
		piob->PIO_CODR = flags;
	}
	lib_close_base(map_base);
	return ERR_OK;
}

static int piob_write_main(int argc, char* argv[]) {
	if(argc < 3) return piob_write_show_usage(ERR_ARGC, "Not enough arguments for piob-write");
	argc --;
	argv ++;
	if(!strcmp(*argv, "on")) return piob_write_state(1, argc, argv);
	if(!strcmp(*argv, "off")) return piob_write_state(0, argc, argv);
	return piob_write_show_usage(ERR_VAL, "Illegal state value for port");
}

static int piob_read_show_usage(int err, const char* msg) {
	static const char* err_fmt = "%s\nUsage: test piob-read <pin>\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

static int piob_read_main(int argc, char* argv[]) {
	if(argc < 2) return piob_read_show_usage(ERR_ARGC, "Not enough arguments for piob-write");
	argc --;
	argv ++;
	int pin = atoi(*argv);
	int port_bit = lib_piob_from_pin(pin);
	if(port_bit == -1) return piob_read_show_usage(ERR_PIN, "Invalid pin number. Only \"B\" pins are supported");
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	volatile struct AT91S_PIO* piob = PIO_B(map_base);
	unsigned flags = 1 << port_bit;
	piob->PIO_PER = flags;
	piob->PIO_ODR = flags;
	int data_bit = piob->PIO_PDSR & (1 << port_bit);
	printf("pin %d %s\n", pin, data_bit ? "on":"off");
	lib_close_base(map_base);
	return ERR_OK;
}

//TCLK1 == PB6 == PIN9 connected to PB0 == PIN3

static int count_init_show_usage(int err, const char* msg) {
	return err;
}

static int count_init_main(int argc, char* argv[]) {
	return ERR_OK;
}

static int count_read_show_usage(int err, const char* msg) {
	return err;
}

static int count_read_main(int  argc, char* argv[]) {
	return ERR_OK;
}

static int show_usage(int err, const char* msg) {
	const char* err_fmt = "%s\nUsage: test <command> <args>\n"\
	"Commands:\n"\
	"\tsimple-blink\n"\
	"\tpiob-write\n"\
	"\tpiob-read\n"\
	"\tcount-init\n"\
	"\tcount-read\n";
	fprintf(stderr, err_fmt, msg);
	return err;
}

int main(int argc, char* argv[]) {
	if(argc < 2) return show_usage(ERR_ARGC, "Not enough arguments for test");
	argc --;
	argv ++;
	if(!strcmp(*argv, "simple-blink"))	return simple_blink_main(argc, argv);
	if(!strcmp(*argv, "piob-write"))	return piob_write_main(argc, argv);
	if(!strcmp(*argv, "piob-read"))		return piob_read_main(argc, argv);
	if(!strcmp(*argv, "count-init"))	return count_init_main(argc, argv);
	if(!strcmp(*argv, "count-read"))	return count_read_main(argc, argv);
	return show_usage(ERR_CMD, "Unknown subcommand");
}

