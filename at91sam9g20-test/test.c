#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "at91sam9g20.h"

#include "bmp180.c"
#include "si7021.c"

enum {
	ERR_OK = 0,
	ERR_ARGC,
	ERR_CMD,
	ERR_MMAP,
	ERR_PIN,
	ERR_VAL,
	ERR_RESET,
};



//http://forum.lazarus.freepascal.org/index.php?topic=21907.0
//https://www.fbi.h-da.de/fileadmin/personal/m.pester/mps/Termin2/Termin2.pdf
//http://www.keil.com/dd/docs/arm/atmel/sam9g20/at91sam9g20.h
//http://forum.arduino.cc/index.php?topic=258619.0
//https://code.google.com/p/embox/source/browse/trunk/embox/src/include/drivers/at91sam7_tcc.h?spec=svn2952&r=2952

#define PIO_B(_b)		((AT91S_PIO*)(_b + 0x600))
#define PMC(_b)			((AT91S_PMC*)(_b + 0xC00))

#define MAP_SIZE		4096UL

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

static void w1_write_0(volatile AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(60);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(4);
}

static void w1_write_1(volatile AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(4);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(60);
}

static unsigned w1_read(volatile AT91S_PIO* piob, unsigned flags) {
	piob->PIO_OER = flags; //enable output
	piob->PIO_CODR = flags; //level low
	lib_delay_us(4);
	piob->PIO_ODR = flags; //disable output
	lib_delay_us(10);
	unsigned bit_value = piob->PIO_PDSR & flags;
	lib_delay_us(45);
	return bit_value;
}

static unsigned w1_read_byte(volatile AT91S_PIO* piob, unsigned flags) {
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

static void w1_write_byte(volatile AT91S_PIO* piob, unsigned flags, unsigned data) {
	data & 0x01 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x02 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x04 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x08 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x10 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x20 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x40 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
	data & 0x80 ? w1_write_1(piob, flags) : w1_write_0(piob, flags);
}

static unsigned ds18b20_reset(volatile AT91S_PIO* piob, unsigned flags) {
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
static volatile AT91S_PIO* io_port_b = 0;
static volatile AT91S_TCB* tcb_base = 0;

//=============================================================================

static int (*log)(FILE*, const char*, ...) = fprintf;
static int empty_log(FILE* ff, const char* fmt, ...) {
	(void)ff;
	(void)fmt;
	return 0;
}

static void shell_gpio() {
	static const char* msg_usage = "gpio pin [enable | disable | input | output | pullup | 1 | 0 | read]\n";
	char* pin_name = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	int port_bit = lib_piob_from_pin(atoi(pin_name));
	if(port_bit == -1) {
		fprintf(stderr, "Invalid pin number (%s). Only GPIO B is supported. %s\n", pin_name, msg_usage);
		return;
	}
	int flags = 1 << port_bit;
	if(!strcmp(pin_action, "enable"))	{ io_port_b->PIO_PER = flags; return; }
	if(!strcmp(pin_action, "disable"))	{ io_port_b->PIO_PDR = flags; return; }
	if(!strcmp(pin_action, "input"))	{ io_port_b->PIO_ODR = flags; return; }
	if(!strcmp(pin_action, "output"))	{ io_port_b->PIO_OER = flags; return; }
	if(!strcmp(pin_action, "pullup"))	{ io_port_b->PIO_PPUER = flags; return; }
	if(!strcmp(pin_action, "1"))		{ io_port_b->PIO_SODR = flags; return; }
	if(!strcmp(pin_action, "0"))		{ io_port_b->PIO_CODR = flags;	return; }
	if(!strcmp(pin_action, "read"))	{
		int data_bit = io_port_b->PIO_PDSR & flags;
		printf("\'%c\' ", data_bit ? '1' : '0');
		return;
	}
}

//=============================================================================
static void shell_ds18b20_presense(int flags) {
	io_port_b->PIO_PER = flags;
	io_port_b->PIO_PPUER = flags; //enable pull up
	unsigned state = ds18b20_reset(io_port_b, flags);
	printf("\'%c\' ", state ? '0' : '1');
}

static void shell_ds18b20_convert(int flags) {
	ds18b20_reset(io_port_b, flags);
	w1_write_byte(io_port_b, flags, DS18B20_SKIP_ROM);
	w1_write_byte(io_port_b, flags, DS18B20_CONVERT_T);
}

static void shell_ds18b20_read(int flags) {
	ds18b20_reset(io_port_b, flags);
	w1_write_byte(io_port_b, flags, DS18B20_SKIP_ROM);
	w1_write_byte(io_port_b, flags, DS18B20_READ_SCRATCHPAD);
	float temp = w1_read_byte(io_port_b, flags) | (w1_read_byte(io_port_b, flags) << 8);
	//skip reading the rest of scratchpad bytes
	io_port_b->PIO_OER = flags; //enable output
	io_port_b->PIO_CODR = flags; //level low
	temp /= 16;
	printf("\'%g\' ", temp);
	usleep(1000);
	io_port_b->PIO_ODR = flags; //disable output
}

static void shell_ds18b20() {
	static const char* msg_usage = "ds18b20 pin [presense | convert | read]\n";
	char* pin_name = strtok(0, SHELL_CMD_DELIMITER);
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	int port_bit = lib_piob_from_pin(atoi(pin_name));
	if(port_bit == -1) {
		fprintf(stderr, "Invalid pin number (%s). Only GPIO B is supported. %s\n", pin_name, msg_usage);
		return;
	}
	int flags = 1 << port_bit;
	if(!strcmp(pin_action, "presense"))	{ shell_ds18b20_presense(flags); return; }
	if(!strcmp(pin_action, "convert"))	{ shell_ds18b20_convert(flags); return; }
	if(!strcmp(pin_action, "read"))		{ shell_ds18b20_read(flags); return; }
}

//=============================================================================

static int shell_counter_init() {
	PMC(io_map_base)->PMC_PCER = (1 << AT91C_ID_TC0); //start periferial clock
	tcb_base->TCB_TC0.TC_IDR = 0xFF;//disable all interrupts for TC0
	tcb_base->TCB_TC0.TC_CMR = AT91C_TC_CLKS_XC1 | AT91C_TC_ETRGEDG_RISING; //XC1 as clock, rising edge
	tcb_base->TCB_BMR = AT91C_TCB_TC1XC1S_TCLK1; //connect XC1 to TCLK1 (pin 9)
	tcb_base->TCB_TC0.TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG; //enable clock, reset counter
	return ERR_OK;
}

static void shell_counter() {
	static const char* msg_usage = "counter [init | read]\n";
	char* pin_action = strtok(0, SHELL_CMD_DELIMITER);
	if(!pin_action) return (void)fprintf(stderr, "%s\n", msg_usage);
	if(!strcmp(pin_action, "init"))		{ shell_counter_init(); return; }
	if(!strcmp(pin_action, "read"))		{ printf("%d ", tcb_base->TCB_TC0.TC_CV); return; }
}

//=============================================================================

static void shell_lm75_read(int fd) {
	float temp;
	char buff[2] = {0};
	if(1 != write(fd, buff, 1) || 2 != read(fd, buff, 2)) {
		fprintf(stderr, "lm75 temperature read failed\n");
		return;
	}
	temp = (float)((short)buff[0] << 8 | buff[1]) / 256;
	printf("\'%g\' ", temp);
}

//i2c pins: 17,18 
static void shell_lm75() {
	static const char* msg_usage = "lm75 address read\n";
	char* address = strtok(0, SHELL_CMD_DELIMITER);
	char* action = strtok(0, SHELL_CMD_DELIMITER);
	if(!action) return (void)fprintf(stderr, "%s\n", msg_usage);
	int addr_i = -1;
	sscanf(address, "%i", &addr_i);
	if(addr_i < 0 || addr_i > 0x77) {
		fprintf(stderr, "Invalid i2c device addressi (%s). Must be between 0x00 and 0x77. %s\n", address, msg_usage);
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

///////////////////////////////////////////////////////////////////////////////
static void shell_bmp180_temperature() {
	struct bmp180_t *bmp180;
	uint8_t err;
	char *string;
	uint32_t pold, pmed;
	uint32_t tcurrent;
	 
	string = malloc(80);
	bmp180 = malloc(sizeof(struct bmp180_t));
	 
	fd = open("/dev/i2c-0", O_RDWR ); //"/dev/i2c-0"
	
	if( ioctl( fd, I2C_SLAVE, BMP180_ADDR ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}
	
	err = bmp180_init(bmp180); 
	
	bmp180->oss = BMP180_RES_ULTRAHIGH;
	print_struct(bmp180, string);
	//printf("  Print Struct Called \n");
	
	err = bmp180_read_all(bmp180);
	pmed = bmp180->p;
	pold = pmed;
	tcurrent = bmp180->T;
	
	if (!err)
	{
			print_results(bmp180, string);
			//printf("  Print Results \n");
	}
	
//	while(1){
//		_delay_ms(1000);
		//err = bmp180_read_pressure(bmp180);
		err = bmp180_read_temperature(bmp180);
		
		/*
		 * pmed = ((pmed * 7) + bmp180->p) >> 3;
		 */
		pmed = bmp180->p;
		tcurrent = bmp180->T;
		
////		sprintf(string,"%u",pmed);
////		printf("pmed is  : %s \n",string);
//		
//		if (abs(pold - pmed) > 1) {
//			pold = pmed;
//			//string = ultoa(pmed, string, 10);
//			sprintf(string,"%u",pmed);
//			printf("PRESSURE : %s \n",string);
//		}
		
		sprintf(string,"%f",(double)(tcurrent/10.0));
		double tmpTempDouble = tcurrent/10.0;
		//printf("TEMPERATURE : %s \n",string);
		//printf("%lu\t%s\t%g\n", time(0),"board1.bmp180.temperature", tmpTempDouble);
		printf("\'%g\'\n", tmpTempDouble);
		
//	}
}

static void shell_bmp180_pressure() {
	struct bmp180_t bmp180;
	uint8_t err;
	char string[80];
	uint32_t pold, pmed;
	uint32_t tcurrent;
	 
	 
	fd = open("/dev/i2c-0", O_RDWR ); //"/dev/i2c-0"
	
	if( ioctl( fd, I2C_SLAVE, BMP180_ADDR ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}
	
	err = bmp180_init(&bmp180); 
	
	bmp180.oss = BMP180_RES_ULTRAHIGH;
	print_struct(&bmp180, string);
	//printf("  Print Struct Called \n");
	
	err = bmp180_read_all(&bmp180);
	pmed = bmp180.p;
	pold = pmed;
	tcurrent = bmp180.T;
	
	if (!err)
	{
			print_results(&bmp180, string);
			//printf("  Print Results \n");
	}
	
		err = bmp180_read_pressure(&bmp180);
		//err = bmp180_read_temperature(&bmp180);
		
		/*
		 * pmed = ((pmed * 7) + bmp180.p) >> 3;
		 */
		pmed = bmp180.p;
		//tcurrent = bmp180.T;
		
	
		
		pold = pmed;
		sprintf(string,"%u",pmed);
		unsigned long tmpLongPressure = pmed;
		//printf("PRESSURE : %s \n",string);
		
		//printf("%lu\t%s\t%lu\n", time(0),"board1.bmp180.pressure", tmpLongPressure);
		printf("\'%lu\'\n", tmpLongPressure);
		
//		sprintf(string,"%f",(double)(tcurrent/10.0));
//		double tmpTempDouble = tcurrent/10.0;
//		//printf("TEMPERATURE : %s \n",string);
//		printf("%lu\t%s\t%g\n", time(0),"board1.bmp180.temperature", tmpTempDouble);
		
}

static void shell_si7021_humidity() {
	float tmpFloatHumidity;
	
	tmpFloatHumidity = getHumidity();

	printf("\'%f\'\n", tmpFloatHumidity);
}
///////////////////////////////////////////////////////////////////////////////

static void shell_ctrl_c(int sig) {
	(void)sig;
	fclose(stdin);
}

int main(int argc, char* argv[]) {
	if(argc > 1 && !strcmp(argv[1], "-q")) log = empty_log;
	io_map_base = lib_open_base((off_t)AT91C_BASE_AIC);
	if(!io_map_base) {
		perror("io_map_base - mmap");
		return ERR_MMAP;
	}
	tcb_base = lib_open_base((off_t)AT91C_BASE_TC0);
	if(!tcb_base) {
		lib_close_base(io_map_base);
		perror("tcp_base - mmap");
		return ERR_MMAP;
	}
	io_port_b = PIO_B(io_map_base);
	char line[SHELL_LINE_BUFF_SIZE];
	log(stderr, "Exit: ctrl+d, help: empty string\n");
	signal(SIGINT, shell_ctrl_c);
	while(fgets(line, sizeof(line), stdin)) {
		char* cmd = strtok(line, SHELL_CMD_DELIMITER);
		if(!cmd) {
			log(stderr, "gpio, counter, ds18b20, lm75 <args>, ctlr+d to exit, .<any text> - comment, empty for help\n");
			continue;
		}
		for(; cmd; cmd = strtok(0, SHELL_CMD_DELIMITER)) {
			if(!strcmp(cmd, "gpio"))	{shell_gpio(); continue;}
			if(!strcmp(cmd, "counter"))	{shell_counter(); continue;}
			if(!strcmp(cmd, "ds18b20"))	{shell_ds18b20(); continue;}
			if(!strcmp(cmd, "lm75"))	{shell_lm75(); continue;}
			if(!strcmp(cmd, "bmp180"))	{shell_bmp180_temperature(); continue;}
			if(!strcmp(cmd, "bmp180p"))	{shell_bmp180_pressure(); continue;}
			if(!strcmp(cmd, "si7021h"))	{shell_si7021_humidity(); continue;}
			printf("%s ", cmd);
		}
		printf("\n");
	}
	lib_close_base(io_map_base);
	lib_close_base(tcb_base);
	log(stderr, "\nshell: cleaning up, exiting.\n");
	return ERR_OK;
}

