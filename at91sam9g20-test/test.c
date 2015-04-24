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
	ERR_RESET,
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

typedef volatile unsigned AT91_REG;

//*****************************************************************************
//** GPIO
//*****************************************************************************
#define PIO_BASE		0xfffff000 //Input Output base address
#define PIO_B(_b)		((struct AT91S_PIO*)(_b + 0x600))

struct AT91S_PIO {
	AT91_REG PIO_PER;       // PIO Enable Register
	AT91_REG PIO_PDR;       // PIO Disable Register
	AT91_REG PIO_PSR;       // PIO Status Register
	AT91_REG Reserved0[1];  //
	AT91_REG PIO_OER;       // Output Enable Register
	AT91_REG PIO_ODR;       // Output Disable Registerr
	AT91_REG PIO_OSR;       // Output Status Register
	AT91_REG Reserved1[1];  //
	AT91_REG PIO_IFER;      // Input Filter Enable Register
	AT91_REG PIO_IFDR;      // Input Filter Disable Register
	AT91_REG PIO_IFSR;      // Input Filter Status Register
	AT91_REG Reserved2[1];  //
	AT91_REG PIO_SODR;      // Set Output Data Register
	AT91_REG PIO_CODR;      // Clear Output Data Register
	AT91_REG PIO_ODSR;      // Output Data Status Register
	AT91_REG PIO_PDSR;      // Pin Data Status Register
	AT91_REG PIO_IER;       // Interrupt Enable Register
	AT91_REG PIO_IDR;       // Interrupt Disable Register
	AT91_REG PIO_IMR;       // Interrupt Mask Register
	AT91_REG PIO_ISR;       // Interrupt Status Register
	AT91_REG PIO_MDER;      // Multi-driver Enable Register
	AT91_REG PIO_MDDR;      // Multi-driver Disable Register
	AT91_REG PIO_MDSR;      // Multi-driver Status Register
	AT91_REG Reserved3[1];  //
	AT91_REG PIO_PPUDR;     // Pull-up Disable Register
	AT91_REG PIO_PPUER;     // Pull-up Enable Register
	AT91_REG PIO_PPUSR;     // Pull-up Status Register
	AT91_REG Reserved4[1];  //
	AT91_REG PIO_ASR;       // Select A Register
	AT91_REG PIO_BSR;       // Select B Register
	AT91_REG PIO_ABSR;      // AB Select Status Register
	AT91_REG Reserved5[9];  //
	AT91_REG PIO_OWER;      // Output Write Enable Register
	AT91_REG PIO_OWDR;      // Output Write Disable Register
	AT91_REG PIO_OWSR;      // Output Write Status Register
};

#define MAP_SIZE		4096UL

//*****************************************************************************
//** Timer Counter
//*****************************************************************************
#define TCB_BASE		0xFFFA0000 //Timer Counter Block base address

struct AT91S_TC {
	AT91_REG		TC_CCR;		// Channel Control Register
	AT91_REG		TC_CMR;		// Channel Mode Register (Capture Mode / Waveform Mode)
	AT91_REG		Reserved0[2];//
	AT91_REG		TC_CV;		// Counter Value
	AT91_REG		TC_RA;		// Register A
	AT91_REG		TC_RB;		// Register B
	AT91_REG		TC_RC;		// Register C
	AT91_REG		TC_SR;		// Status Register
	AT91_REG		TC_IER;		// Interrupt Enable Register
	AT91_REG		TC_IDR;		// Interrupt Disable Register
	AT91_REG		TC_IMR;		// Interrupt Mask Register
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
#define TC_ENETRG					(0x1 << 12)		//TC External Event Trigger enable
#define TC_EEVTEDG_RISING			(0x1 <<  8)		//TC Edge: rising edge
#define TC_EEVT_XC1					(0x2 << 10)		//TC Signal selected as external event: XC1 TIOB direction: output
#define TC_ABETRG					(0x1 << 10)		//TC TIOA or TIOB External Trigger Selection

struct AT91S_TCB {
	struct AT91S_TC		TCB_TC0;       // TC Channel 0
	AT91_REG			Reserved0[4];  //
	struct AT91S_TC		TCB_TC1;       // TC Channel 1
	AT91_REG			Reserved1[4];  //
	struct AT91S_TC		TCB_TC2;       // TC Channel 2
	AT91_REG			Reserved2[4];  //
	AT91_REG			TCB_BCR;       // TC Block Control Register
	AT91_REG			TCB_BMR;       // TC Block Mode Register
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


//SOFTWARE API DEFINITION  FOR Power Management Controler
#define PMC(_b)			((struct AT91S_PMC*)(_b + 0xC00))
struct AT91S_PMC {
	AT91_REG		PMC_SCER;			// System Clock Enable Register
	AT91_REG		PMC_SCDR;			// System Clock Disable Register
	AT91_REG		PMC_SCSR;			// System Clock Status Register
	AT91_REG		Reserved0[1];		// 
	AT91_REG		PMC_PCER;			// Peripheral Clock Enable Register
	AT91_REG		PMC_PCDR;			// Peripheral Clock Disable Register
	AT91_REG		PMC_PCSR;			// Peripheral Clock Status Register
	AT91_REG		Reserved1[1];		// 
	AT91_REG		PMC_MOR;			// Main Oscillator Register
	AT91_REG		PMC_MCFR;			// Main Clock  Frequency Register
	AT91_REG		PMC_PLLAR;			// PLL A Register
	AT91_REG		PMC_PLLBR;			// PLL B Register
	AT91_REG		PMC_MCKR;			// Master Clock Register
	AT91_REG		Reserved2[3];		// 
	AT91_REG		PMC_PCKR[8];		// Programmable Clock Register
	AT91_REG		PMC_IER;			// Interrupt Enable Register
	AT91_REG		PMC_IDR;			// Interrupt Disable Register
	AT91_REG		PMC_SR;				// Status Register
	AT91_REG		PMC_IMR;			// Interrupt Mask Register
};

//PERIPHERAL ID DEFINITIONS FOR AT91SAM9260A
#define AT91C_ID_TC0				(17)						// Timer Counter 0
#define AT91C_ID_TC1				(18)						// Timer Counter 1
#define AT91C_ID_TC2				(19)						// Timer Counter 2


void lib_delay_uS(unsigned period) {
	volatile unsigned i = 79 * period / 2;
	while(i --);
}

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
	if(argc < 2) return piob_read_show_usage(ERR_ARGC, "Not enough arguments for piob-read");
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	volatile struct AT91S_PIO* piob = PIO_B(map_base);
	int cnt = 0;
	while(--argc) {
		int pin = atoi(*++argv);
		int port_bit = lib_piob_from_pin(pin);
		if(port_bit == -1) {
			fprintf(stderr, "Invalid pin number: %s. Only \"B\" pins are supported\n", *argv);
			continue;
		}
		cnt ++;
		int flags = 1 << port_bit;
		piob->PIO_PER = flags;
		piob->PIO_ODR = flags;
		int data_bit = piob->PIO_PDSR & flags;
		printf("%d %s ", pin, data_bit ? "on":"off");
	}
	lib_close_base(map_base);
	if(cnt) {
		puts("");
	} else {
		return piob_read_show_usage(ERR_PIN, "No pins selected - nothing to do");
	}
	return ERR_OK;
}

//TCLK1 == PB6 == PIN9 connected to PB0 == PIN3

static int count_init_show_usage(int err, const char* msg) {
	return err;
}

static int count_init_main(int argc, char* argv[]) {
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	struct AT91S_PMC* pmc = PMC(map_base);
	pmc->PMC_PCER = (1 << AT91C_ID_TC0); //start periferial clock
	lib_close_base(map_base);
	volatile struct AT91S_TCB *tcb = lib_open_base(TCB_BASE);
	if(!tcb) return ERR_MMAP;
	tcb->TCB_TC0.TC_IDR = 0xFF;//disable all interrupts for TC0
	tcb->TCB_TC0.TC_CMR = TC_CLKS_XC1 | TC_ETRGEDG_RISING; //XC1 as clock, rising edge
	tcb->TCB_BMR = AT91C_TCB_TC1XC1S_TCLK1; //connect XC1 to TCLK1 (pin 3)
	tcb->TCB_TC0.TC_CCR = TC_CLKEN | TC_SWTRG; //enable clock, reset counter
	lib_close_base(tcb);
	return ERR_OK;
}

static int count_read_show_usage(int err, const char* msg) {
	return err;
}

static int count_read_main(int  argc, char* argv[]) {
	volatile struct AT91S_TCB *tcb = lib_open_base(TCB_BASE);
	if(!tcb) return ERR_MMAP;
	printf("counter %d\n", tcb->TCB_TC0.TC_CV);
	lib_close_base(tcb);
	return ERR_OK;
}

static void w1_write_0(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_uS(60);
	piob->PIO_ODR = flags; //disable output
	lib_delay_uS(60);
}

static void w1_write_1(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_uS(1);
	piob->PIO_ODR = flags; //disable output
	lib_delay_uS(120);
}

static unsigned w1_read(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_uS(1);
	piob->PIO_ODR = flags; //disable output
	lib_delay_uS(2);
	unsigned bit_value = piob->PIO_PDSR & flags;
	lib_delay_uS(60);
	return bit_value;
}

static unsigned w1_read_byte(volatile struct AT91S_PIO* piob, unsigned flags) {
	unsigned result = 0;
	if(w1_read(piob, flags)) result |= 0x01;
	if(w1_read(piob, flags)) result |= 0x02;
	if(w1_read(piob, flags)) result |= 0x04;
	if(w1_read(piob, flags)) result |= 0x08;
	if(w1_read(piob, flags)) result |= 0x10;
	if(w1_read(piob, flags)) result |= 0x20;
	if(w1_read(piob, flags)) result |= 0x40;
	if(w1_read(piob, flags)) result |= 0x80;
	return result;
}

static void w1_write_byte(volatile struct AT91S_PIO* piob, unsigned flags, unsigned data) {
	data & 0x01 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x02 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x04 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x08 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x10 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x20 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x40 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x80 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	lib_delay_uS(100);
}

static unsigned ds18b20_reset(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_PPUER = flags; //enable pull up
	piob->PIO_PER = flags; //enable pin 10
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_uS(500);
	piob->PIO_ODR = flags; //disable output
	lib_delay_uS(60);
	int data_bit = piob->PIO_PDSR;
	lib_delay_uS(480);
	return data_bit & flags;
}

#define DS18B20_READ_ROM			0x33
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_READ_SCRATCHPAD		0xBE
#define DS18B20_CONVERT_T			0x44

static void ds18b20_read_rom(volatile struct AT91S_PIO* piob, unsigned flags) {
	unsigned char buff[8];
	w1_write_byte(piob, flags, DS18B20_READ_ROM);
	buff[0] = w1_read_byte(piob, flags);
	buff[1] = w1_read_byte(piob, flags);
	buff[2] = w1_read_byte(piob, flags);
	buff[3] = w1_read_byte(piob, flags);
	buff[4] = w1_read_byte(piob, flags);
	buff[5] = w1_read_byte(piob, flags);
	fprintf(stderr, ">>>> %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7]);
}

static void ds18b20_read_scratchpad(volatile struct AT91S_PIO* piob, unsigned flags) {
	unsigned char buff[9];
	w1_write_byte(piob, flags, DS18B20_READ_SCRATCHPAD);
	buff[0] = w1_read_byte(piob, flags);
	buff[1] = w1_read_byte(piob, flags);
	buff[2] = w1_read_byte(piob, flags);
	buff[3] = w1_read_byte(piob, flags);
	buff[4] = w1_read_byte(piob, flags);
	buff[5] = w1_read_byte(piob, flags);
	buff[6] = w1_read_byte(piob, flags);
	buff[7] = w1_read_byte(piob, flags);
	buff[8] = w1_read_byte(piob, flags);
	fprintf(stderr, ">>>> %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], buff[8]);
}

static int temp_read_main(int  argc, char* argv[]) {
	nice(-10);
	volatile void* map_base = lib_open_base(PIO_BASE);
	if(!map_base) return ERR_MMAP;
	volatile struct AT91S_PIO* piob = PIO_B(map_base);
	int port_bit = lib_piob_from_pin(7);
	unsigned flags = 1 << port_bit;
	unsigned state = ds18b20_reset(piob, flags);
	if(state) {
		fprintf(stderr, "Error reseting device: %u\n", state);
		lib_close_base(map_base);
		return ERR_RESET;
	}
	ds18b20_read_rom(piob, flags);
	w1_write_byte(piob, flags, DS18B20_SKIP_ROM);
	w1_write_byte(piob, flags, DS18B20_CONVERT_T);
	sleep(1);
	ds18b20_read_scratchpad(piob, flags);
	lib_close_base(map_base);
	return ERR_OK;
}

static int show_usage(int err, const char* msg) {
	const char* err_fmt = "%s\nUsage: test <command> <args>\n"\
	"Commands:\n"\
	"\tsimple-blink\n"\
	"\tpiob-write\n"\
	"\tpiob-read\n"\
	"\tcount-init\n"\
	"\tcount-read\n"\
	"\ttemp-read\n";
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
	if(!strcmp(*argv, "temp-read"))		return temp_read_main(argc, argv);
	return show_usage(ERR_CMD, "Unknown subcommand");
}

