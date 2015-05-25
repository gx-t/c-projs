#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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
	if(!map_base) return;
	munmap((void*)map_base, MAP_SIZE);
}

// calibrated with good accuracy for GESBC-9G20u board
void lib_delay_us(unsigned us) {
	volatile unsigned i = 79 * us / 2;
	while(i --);
}

// 1 wire timings from here:
// http://en.wikipedia.org/wiki/1-Wire
// checked playing with read delays - got stable result for wide range

static void w1_write_0(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(60);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(4);
}

static void w1_write_1(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(4);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(60);
}

static unsigned w1_read(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(4);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(10);
	unsigned bit_value = piob->PIO_PDSR & flags;
	lib_delay_us(45);
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
}

static unsigned ds18b20_reset(volatile struct AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(500);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(60);
	int data_bit = piob->PIO_PDSR;
	usleep(1000);
	return data_bit & flags;
}

#define DS18B20_READ_ROM			0x33
#define DS18B20_SKIP_ROM			0xCC
#define DS18B20_READ_SCRATCHPAD		0xBE
#define DS18B20_CONVERT_T			0x44

//==============================================
#define SHELL_LINE_BUFF_SIZE		4096
#define SHELL_CMD_DELIMITER			" \t\r\n"

static volatile void* io_map_base = 0;
static volatile struct AT91S_PIO* io_port_b = 0;
static volatile struct AT91S_TCB* tcb_base = 0;

//=============================================================================

static void shell_gpio() {
	static const char* msg_usage = "gpio context pin [enable | disable | input | output | 1 | 0 | read]\n";
	char* context = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_name = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	int port_bit = lib_piob_from_pin(atoi(pin_name));
	if(port_bit == -1) {
		fprintf(stderr, "Invalid pin number (%s). Only GPIO B is supported. %s\n", pin_name, msg_usage);
		return;
	}
	int flags = 1 << port_bit;
	for(; pin_action; pin_action = strtok(0, SHELL_CMD_DELIMITER)) {
		if(!strcmp(pin_action, "enable"))	{ io_port_b->PIO_PER = flags; continue; }
		if(!strcmp(pin_action, "disable"))	{ io_port_b->PIO_PDR = flags; continue; }
		if(!strcmp(pin_action, "input"))	{ io_port_b->PIO_ODR = flags; continue; }
		if(!strcmp(pin_action, "output"))	{ io_port_b->PIO_OER = flags; continue; }
		if(!strcmp(pin_action, "1"))		{ io_port_b->PIO_SODR = flags; continue; }
		if(!strcmp(pin_action, "0"))		{ io_port_b->PIO_CODR = flags;	continue; }
		if(!strcmp(pin_action, "read"))	{
			int data_bit = io_port_b->PIO_PDSR & flags;
			printf("%lu\t%s\t%c\n", time(0), context, data_bit ? '1' : '0');
			continue;
		}
		fprintf(stderr, "Unknown action: %s, ignoring. %s\n", pin_action, msg_usage);
	}
}

//=============================================================================

static void shell_ds18b20_presense(int flags, char* context) {
	io_port_b->PIO_PER = flags;
	io_port_b->PIO_PPUER = flags; //enable pull up
	unsigned state = ds18b20_reset(io_port_b, flags);
	printf("%lu\t%s\t%c\n", time(0), context, state ? '0' : '1');
}

static void shell_ds18b20_convert(int flags) {
	ds18b20_reset(io_port_b, flags);
	w1_write_byte(io_port_b, flags, DS18B20_SKIP_ROM);
	w1_write_byte(io_port_b, flags, DS18B20_CONVERT_T);
}

static void shell_ds18b20_read(int flags, char* context) {
	ds18b20_reset(io_port_b, flags);
	w1_write_byte(io_port_b, flags, DS18B20_SKIP_ROM);
	w1_write_byte(io_port_b, flags, DS18B20_READ_SCRATCHPAD);
	float temp = w1_read_byte(io_port_b, flags) | (w1_read_byte(io_port_b, flags) << 8);
	//skip reading the rest of scratchpad bytes
	io_port_b->PIO_OER = flags; //enable output
	io_port_b->PIO_CODR = flags; //level low
	temp /= 16;
	printf("%lu\t%s\t%g\n", time(0), context, temp);
	usleep(1000);
	io_port_b->PIO_ODR = flags; //disable output
}

static void shell_ds18b20() {
	static const char* msg_usage = "ds18b20 context pin [presense | convert | read]\n";
	char* context = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_name = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	int port_bit = lib_piob_from_pin(atoi(pin_name));
	if(port_bit == -1) {
		fprintf(stderr, "Invalid pin number (%s). Only GPIO B is supported. %s\n", pin_name, msg_usage);
		return;
	}
	int flags = 1 << port_bit;
	for(; pin_action; pin_action = strtok(0, SHELL_CMD_DELIMITER)) {
		if(!strcmp(pin_action, "presense"))	{ shell_ds18b20_presense(flags, context); continue; }
		if(!strcmp(pin_action, "convert"))	{ shell_ds18b20_convert(flags); continue; }
		if(!strcmp(pin_action, "read"))		{ shell_ds18b20_read(flags, context); continue; }
		fprintf(stderr, "Unknown action: %s, ignoring. %s\n", pin_action, msg_usage);
	}
}

//=============================================================================

static int shell_counter_init() {
	PMC(io_map_base)->PMC_PCER = (1 << AT91C_ID_TC0); //start periferial clock
	tcb_base->TCB_TC0.TC_IDR = 0xFF;//disable all interrupts for TC0
	tcb_base->TCB_TC0.TC_CMR = TC_CLKS_XC1 | TC_ETRGEDG_RISING; //XC1 as clock, rising edge
	tcb_base->TCB_BMR = AT91C_TCB_TC1XC1S_TCLK1; //connect XC1 to TCLK1 (pin 9)
	tcb_base->TCB_TC0.TC_CCR = TC_CLKEN | TC_SWTRG; //enable clock, reset counter
	return ERR_OK;
}

static void shell_counter() {
	static const char* msg_usage = "counter context [init | read]\n";
	char* context = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	for(; pin_action; pin_action = strtok(0, SHELL_CMD_DELIMITER)) {
		if(!strcmp(pin_action, "init"))		{ shell_counter_init(); continue; }
		if(!strcmp(pin_action, "read"))		{ printf("%lu\t%s\t%d\n", time(0), context, tcb_base->TCB_TC0.TC_CV); continue; }
		fprintf(stderr, "Unknown action: %s, ignoring. %s\n", pin_action, msg_usage);
	}
}

//=============================================================================

static void shell_lm75_read(int fd) {
	char buff[2];
	buff[0] = 0;
	if(1 != write(fd, buff, 1) || 2 != read(fd, buff, 2)) {
		fprintf(stderr, "lm75 temperature read failed\n");
		return;
	}
	printf("%g\n", (float)((short)buff[0] << 8 | buff[1]) / 256);
}

static void shell_lm75() {
	static const char* msg_usage = "lm75 context address read\n";
	char* context = strtok(0, SHELL_CMD_DELIMITER);
	char* address = strtok(0, SHELL_CMD_DELIMITER);
	char* action = strtok(0, SHELL_CMD_DELIMITER);
	int addr_i = -1;
	sscanf(address, "%i", &addr_i);
	if(addr_i < 0 || addr_i > 0x77) {
		fprintf(stderr, "Invalid i2c device addressi (%s). Must be between 0x00 and 0x77\n", address);
		return;
	}
	int fd = open("/dev/i2c-0", O_RDWR);
	if(fd < 0) {
		perror("/dev/i2c-0");
		return;
	}
	do {
		if(ioctl(fd, I2C_SLAVE, addr_i) < 0) {
			perror("Failed to acquire slave address");
			break;
		}
		shell_lm75_read(fd);
	}while(0);
	close(fd);
}

static void shell_ctrl_c(int sig) {
	(void)sig;
	fclose(stdin);
}

int main() {
	io_map_base = lib_open_base(PIO_BASE);
	if(!io_map_base) {
		perror("io_map_base - mmap");
		return ERR_MMAP;
	}
	tcb_base = lib_open_base(TCB_BASE);
	if(!tcb_base) {
		lib_close_base(io_map_base);
		perror("tcp_base - mmap");
		return ERR_MMAP;
	}
	io_port_b = PIO_B(io_map_base);
	char line[SHELL_LINE_BUFF_SIZE];
	fprintf(stderr, "Exit: ctrl+d, help: empty string\n");
	signal(SIGINT, shell_ctrl_c);
	while(fgets(line, sizeof(line), stdin)) {
		if(*line == '.') {printf("%s", line + 1); continue;}
		char* cmd = strtok(line, SHELL_CMD_DELIMITER);
		if(!cmd) {
			fprintf(stderr, "gpio, counter, ds18b20, lm75 <args>, ctlr+d to exit, .<any text> - comment, empty for help\n");
			continue;
		}
		if(!strcmp(cmd, "gpio"))	{shell_gpio(); continue;}
		if(!strcmp(cmd, "counter"))	{shell_counter(); continue;}
		if(!strcmp(cmd, "ds18b20"))	{shell_ds18b20(); continue;}
		if(!strcmp(cmd, "lm75"))	{shell_lm75(); continue;}
		fprintf(stderr, "Unknown command: %s\n", cmd);
	}
	lib_close_base(io_map_base);
	lib_close_base(tcb_base);
	fprintf(stderr, "\nshell: cleaning up, exiting.\n");
	return ERR_OK;
}

