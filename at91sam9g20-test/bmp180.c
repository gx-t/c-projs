/*
 * mpl115.c - Support for Freescale MPL115A2 pressure/temperature sensor
 *
 * Copyright (c) 2014 Peter Meerwald <pmeerw@pmeerw.net>
 *
 * This file is subject to the terms and conditions of version 2 of
 * the GNU General Public License.  See the file COPYING in the main
 * directory of this archive for more details.
 *
 * (7-bit I2C slave address 0x60)
 *
 * TODO: shutdown pin
 *
 */

//#include <linux/module.h>
//#include <linux/i2c.h>
//#include <linux/iio/iio.h>
//#include <linux/delay.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/i2c-dev.h>


#define BMP180_REG_AC1 0xaa
#define BMP180_REG_AC2 0xac
#define BMP180_REG_AC3 0xae
#define BMP180_REG_AC4 0xb0
#define BMP180_REG_AC5 0xb2
#define BMP180_REG_AC6 0xb4
#define BMP180_REG_B1 0xb6
#define BMP180_REG_B2 0xb8
#define BMP180_REG_MB 0xba
#define BMP180_REG_MC 0xbc
#define BMP180_REG_MD 0xbe

#define BMP180_REG_ID 0xd0
#define BMP180_REG_CTRL 0xf4
#define BMP180_REG_ADC 0xf6
#define BMP180_REG_ADCMSB 0xf6
#define BMP180_REG_ADCLSB 0xf7
#define BMP180_REG_ADCXLSB 0xf8

#define BMP180_RES_LOW 0
#define BMP180_RES_STD 1
#define BMP180_RES_HIGH 2
#define BMP180_RES_ULTRAHIGH 3

#define BMP180_ADDR 0x77

struct bmp180_t {
	uint8_t id;

	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;

	uint8_t oss;

	int32_t UT;
	int32_t UP;
	int32_t T;
	int32_t p;

	int32_t B5;

	uint8_t flags;
};

int fd;

//void lib_delay_us(unsigned long us);
void scan_i2c_bus(int file);
uint8_t bmp180_init(struct bmp180_t *bmp180);
uint8_t dump_calibration_data(struct bmp180_t *bmp180);
uint8_t register_rw(const uint8_t reg_addr, uint16_t *word);
uint8_t register_rb(const uint8_t reg_addr, uint8_t *byte);
uint8_t register_wb(const uint8_t reg_addr, uint8_t byte);
uint8_t bmp180_read_pressure(struct bmp180_t *bmp180);
uint8_t bmp180_read_temperature(struct bmp180_t *bmp180);
uint8_t bmp180_read_all(struct bmp180_t *bmp180);
void math_pressure(struct bmp180_t *bmp180);
void math_temperature(struct bmp180_t *bmp180);
uint8_t bmp180_resolution(const uint8_t mode);
void _delay_ms(unsigned ms);

void print_struct(struct bmp180_t *bmp180, char *string);
void print_results(struct bmp180_t *bmp180, char *string);

//int main()
//{
//	struct bmp180_t *bmp180;
//	uint8_t err;
//	char *string;
//	uint32_t pold, pmed;
//	uint32_t tcurrent;
//	 
//	string = malloc(80);
//	bmp180 = malloc(sizeof(struct bmp180_t));
//	 
//	fd = open("/dev/i2c-0", O_RDWR ); //"/dev/i2c-0"
//	
//	if( ioctl( fd, I2C_SLAVE, BMP180_ADDR ) < 0 )
//	{
//		fprintf( stderr, "Failed to set slave address: %m\n" );
//		return 2;
//	}
//	
//	err = bmp180_init(bmp180); 
//	
//	bmp180->oss = BMP180_RES_ULTRAHIGH;
//	print_struct(bmp180, string);
//	//printf("  Print Struct Called \n");
//	
//	err = bmp180_read_all(bmp180);
//	pmed = bmp180->p;
//	pold = pmed;
//	tcurrent = bmp180->T;
//	
//	if (!err)
//	{
//			print_results(bmp180, string);
//			printf("  Print Results \n");
//	}
//	
//	while(1){
//		_delay_ms(1000);
//		err = bmp180_read_pressure(bmp180);
//		err = bmp180_read_temperature(bmp180);
//		
//		/*
//		 * pmed = ((pmed * 7) + bmp180->p) >> 3;
//		 */
//		pmed = bmp180->p;
//		tcurrent = bmp180->T;
//		
////		sprintf(string,"%u",pmed);
////		printf("pmed is  : %s \n",string);
//		
//		if (abs(pold - pmed) > 1) {
//			pold = pmed;
//			//string = ultoa(pmed, string, 10);
//			sprintf(string,"%u",pmed);
//			printf("PRESSURE : %s \n",string);
//		}
//		
//		sprintf(string,"%f",(double)(tcurrent/10.0));
//		printf("TEMPERATURE : %s \n",string);
//		
//	}
//	  
//	return 0;
//}

//---------------------------------------------------------------------------------------------------
void print_struct(struct bmp180_t *bmp180, char *string)
{
	//string = utoa(bmp180->id, string, 16);
	sprintf(string,"id %u",bmp180->id);

	//string = utoa(bmp180->oss, string, 16);
	sprintf(string,"oss %u",bmp180->oss);

	//string = itoa(bmp180->AC1, string, 10);
	sprintf(string,"AC1 %u",bmp180->AC1);
	
	//string = itoa(bmp180->AC2, string, 10);
	sprintf(string,"AC2 %u",bmp180->AC2);

	//string = itoa(bmp180->AC3, string, 10);
	sprintf(string,"AC3 %u",bmp180->AC3);

	//string = utoa(bmp180->AC4, string, 10);
	sprintf(string,"AC4 %u",bmp180->AC4);

	//string = utoa(bmp180->AC5, string, 10);
	sprintf(string,"AC5 %u",bmp180->AC5);

	//string = utoa(bmp180->AC6, string, 10);
	sprintf(string,"AC6 %u",bmp180->AC6);

	//string = itoa(bmp180->B1, string, 10);
	sprintf(string,"B1 %u",bmp180->B1);

	//string = itoa(bmp180->B2, string, 10);
	sprintf(string,"B2 %u",bmp180->B2);

	//string = itoa(bmp180->MB, string, 10);
	sprintf(string,"MB %u",bmp180->MB);

	//string = itoa(bmp180->MC, string, 10);
	sprintf(string,"MC %u",bmp180->MC);

	//string = itoa(bmp180->MD, string, 10);
	sprintf(string,"MD %u",bmp180->MD);
}

void print_results(struct bmp180_t *bmp180, char *string)
{
	//string = ultoa(bmp180->UT, string, 10);
	sprintf(string,"%u",bmp180->UT);

	//string = ultoa(bmp180->UP, string, 10);
	sprintf(string,"%u",bmp180->UP);

	//string = ultoa(bmp180->T, string, 10);
	sprintf(string,"%u",bmp180->T);
	
	//string = ultoa(bmp180->p, string, 10);
	sprintf(string,"%u",bmp180->p);
}
//-----------------------------------------------------------------------------------------

/** Register Read (word).
 *
 * @param reg_addr the register address.
 * @param byte the data to be read.
 */
uint8_t register_rw(const uint8_t reg_addr, uint16_t *word)
{	
	*word =  i2c_smbus_read_word_data(fd,reg_addr);
	//printf("Word : %d \n",*word);
	
	return 0;
}

/** Register Read (Byte).
 *
 * @param reg_addr the register address.
 * @param byte the data to be read.
 */
uint8_t register_rb(const uint8_t reg_addr, uint8_t *byte)
{
	*byte =  i2c_smbus_read_byte_data(fd,reg_addr);
	//printf("Byte : %d \n",*byte);
	
	return 0;
}

/** Register write (Byte).
 *
 * @param reg_addr the register address.
 * @param byte the data to be written.
 */
uint8_t register_wb(const uint8_t reg_addr, uint8_t byte)
{
	uint8_t error;

	error = i2c_smbus_write_byte_data(fd, reg_addr, byte);
	return (error);
}

/** Read the calibration data.
 *
 */
uint8_t dump_calibration_data(struct bmp180_t *bmp180)
{
	uint8_t err;
	uint16_t word;
	uint8_t word_MSB;
	uint8_t word_LSB;
		
	//err = register_rw(BMP180_REG_AC1, &word);
	err = register_rb(BMP180_REG_AC1, &word_MSB);
	err = register_rb(0xAB, &word_LSB);
	word = word_MSB;
	word = word << 8;
	word = word + word_LSB;
	bmp180->AC1 = (int16_t) word;
	//printf("AC1 %d \n",bmp180->AC1);

	if (!err) {
		//err = register_rw(BMP180_REG_AC2, &word);
		err = register_rb(BMP180_REG_AC2, &word_MSB);
		err = register_rb(0xAD, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->AC2 = (int16_t) word;
		//printf("AC2 %d \n",bmp180->AC2);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_AC3, &word);
		err = register_rb(BMP180_REG_AC3, &word_MSB);
		err = register_rb(0xAF, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->AC3 = (int16_t) word;
		//printf("AC3 %d \n",bmp180->AC3);
	}

	if (!err){
		//err = register_rw(BMP180_REG_AC4, &word);
		err = register_rb(BMP180_REG_AC4, &word_MSB);
		err = register_rb(0xB1, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->AC4 = (int16_t) word;
		//printf("AC4 %d \n",bmp180->AC4);
	}

	if (!err){
		//err = register_rw(BMP180_REG_AC5, &word);
		err = register_rb(BMP180_REG_AC5, &word_MSB);
		err = register_rb(0xB3, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->AC5 = (int16_t) word;
		//printf("AC5 %d \n",bmp180->AC5);
	}
	if (!err){
		//err = register_rw(BMP180_REG_AC6, &word);
		err = register_rb(BMP180_REG_AC6, &word_MSB);
		err = register_rb(0xB5, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->AC6 = (int16_t) word;
		//printf("AC6 %d \n",bmp180->AC6);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_B1, &word);
		err = register_rb(BMP180_REG_B1, &word_MSB);
		err = register_rb(0xB7, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->B1 = (int16_t) word;
		//printf("B1 %d \n",bmp180->B1);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_B2, &word);
		err = register_rb(BMP180_REG_B2, &word_MSB);
		err = register_rb(0xB9, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->B2 = (int16_t) word;
		//printf("B2 %d \n",bmp180->B2);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_MB, &word);
		err = register_rb(BMP180_REG_MB, &word_MSB);
		err = register_rb(0xBB, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->MB = (int16_t) word;
		//printf("MB %d \n",bmp180->MB);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_MC, &word);
		err = register_rb(BMP180_REG_MC, &word_MSB);
		err = register_rb(0xBD, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->MC = (int16_t) word;
		//printf("MC %d \n",bmp180->MC);
	}

	if (!err) {
		//err = register_rw(BMP180_REG_MD, &word);
		err = register_rb(BMP180_REG_MD, &word_MSB);
		err = register_rb(0xBF, &word_LSB);
		word = word_MSB;
		word = word << 8;
		word = word + word_LSB;
		bmp180->MD = (int16_t) word;
		//printf("MD %d \n",bmp180->MD);
	}

	return(err);
}

/** Init
 */
uint8_t bmp180_init(struct bmp180_t *bmp180)
{
	uint8_t err;

	bmp180->flags = 0;
	err = register_rb(BMP180_REG_ID, &bmp180->id);
	
	if (!err && (bmp180->id == 0x55)) {
		err = register_rb(BMP180_REG_CTRL, &bmp180->oss);
		bmp180->oss >>= 6;
		//printf("ID == 0x55\n");
		
		if (!err)
		{
			err = dump_calibration_data(bmp180);
		}
	}

	return (err);
}

uint8_t bmp180_read_pressure(struct bmp180_t *bmp180)
{
	uint8_t err, byte;
	uint16_t word;
	uint8_t UP_MSB;
	uint8_t UP_LSB;
	
	/* Read UP */
	err = register_wb(BMP180_REG_CTRL, (0x34 + (bmp180->oss << 6)));

	if (!err) {
		switch (bmp180->oss) {
			case BMP180_RES_LOW:
				_delay_ms(5);
				break;
			case BMP180_RES_STD:
				_delay_ms(8);
				break;
			case BMP180_RES_HIGH:
				_delay_ms(14);
				break;
			default:
				_delay_ms(26);
		}

		//err = register_rw(BMP180_REG_ADC, &word);
		err = register_rb(BMP180_REG_ADC, &UP_MSB);
		err = register_rb(BMP180_REG_ADCLSB, &UP_LSB);
		word = UP_MSB;
		word = word << 8;
		word = word + UP_LSB;
		bmp180->UP = (int32_t)word << 8;

		if (!err) {
			err = register_rb(BMP180_REG_ADCXLSB, &byte);
			bmp180->UP |= byte;
			bmp180->UP >>= (8 - bmp180->oss);
		}

		if (!err)
			math_pressure(bmp180);
	}

	return(err);
}

/** The temperature read and converter.
 *
 * See datasheet for details.
 */
uint8_t bmp180_read_temperature(struct bmp180_t *bmp180)
{
	uint8_t err;
	uint8_t UT_MSB;
	uint8_t UT_LSB;
	uint16_t word;

	/* Read UT */
	err = register_wb(BMP180_REG_CTRL, 0x2e);
	//printf("Temperature command error %d \n",err);
	if (!err) {
		_delay_ms(5);
		//printf("Temperature command written \n");
		//err = register_rw(BMP180_REG_ADC, &word);
		err = register_rb(BMP180_REG_ADC, &UT_MSB);
		err = register_rb(BMP180_REG_ADCLSB, &UT_LSB);
		word = UT_MSB;
		word = word << 8;
		word = word + UT_LSB;
		bmp180->UT = (long)word;
		//printf("UT is : %d \n", bmp180->UT);
		
		if (!err)
			math_temperature(bmp180);
	}

	return (err);
}

uint8_t bmp180_read_all(struct bmp180_t *bmp180)
{
	uint8_t err;
	char* string;
	uint32_t currentTemp;

	err = bmp180_read_temperature(bmp180);

	currentTemp = bmp180->T;
	sprintf(string,"%f",(double)(currentTemp/10));
	//printf("--------------> INITIAL TEMPERATURE : %s \n",string);
			
	if (!err)
	{
		err = bmp180_read_pressure(bmp180);
		//printf("Read pressure called \n");
	}

	return(err);
}

void math_pressure(struct bmp180_t *bmp180)
{
	uint32_t b4, b7;
	int32_t x1, x2, x3, b3, b6;

	b6 = bmp180->B5 - 4000;
	x1 = (bmp180->B2 * (b6 * b6 >> 12)) >> 11;
	x2 = bmp180->AC2 * b6 >> 11;
	x3 = x1 + x2;

	b3 = ((((int32_t)bmp180->AC1 * 4 + x3) << bmp180->oss) + 2) >> 2;
	x1 = bmp180->AC3 * b6 >> 13;
	x2 = (bmp180->B1 * (b6 * b6 >> 12)) >> 16;
	x3 = (x1 + x2 + 2) >> 2;
	b4 = (bmp180->AC4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = (uint32_t)(bmp180->UP - b3) * (50000 >> bmp180->oss);

	if (b7 < 0x80000000)
		bmp180->p = (b7 << 1) / b4;
	else
		bmp180->p = (b7 / b4) << 1;

	x1 = (bmp180->p >> 8) * (bmp180->p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * bmp180->p) >> 16;
	bmp180->p += ((x1 + x2 + 3791) >> 4);
}

uint8_t bmp180_resolution(const uint8_t mode)
{
	uint8_t err, byte;

	err = register_rb(BMP180_REG_CTRL, &byte);

	if (!err) {
		byte &= 0x3f; /* 0b00111111 */
		byte |= (mode << 6);
	}

	return(err);
}

void math_temperature(struct bmp180_t *bmp180)
{
	int32_t x1, x2;
	int err;
	
	err = dump_calibration_data(bmp180);

	x1 = (bmp180->UT - bmp180->AC6) * bmp180->AC5 >> 15;
	//printf("UT is %d \n", bmp180->UT);
	//printf("AC4 is %d \n", bmp180->AC4);
	//printf("AC5 is %d \n", bmp180->AC5);
	//printf("AC6 is %d \n", bmp180->AC6);
	//printf("X1 is %d \n", x1);
	x2 = ((int32_t)bmp180->MC << 11) / (x1 + bmp180->MD);
	//printf("X2 is %d \n", x2);
	bmp180->B5 = x1 + x2;
	//printf("B5 is %d \n", bmp180->T);
	bmp180->T = (bmp180->B5 + 8) >> 4;
	//printf("Math Temperature \n");
	//printf("Mathed Temperature is %d \n", bmp180->T);
}

void _delay_ms(unsigned ms) 
{
	lib_delay_us(ms*1000);
}

////------------------------------------------------------------
//// calibrated with good accuracy for GESBC-9G20u board
//void lib_delay_us(unsigned long us) 
//{
//	volatile unsigned long i = 79 * us / 2;
//	while(i --);
//}

// Search the I2C address space for slaves.
void scan_i2c_bus(int file)
{
  int port;
  
  for (port=0; port<127; port++)
  {
    int res;
    if (ioctl(file, I2C_SLAVE, port) < 0)
      perror("ioctl() I2C_SLAVE failed\n");
    res = i2c_smbus_read_byte(file);
    if (res >= 0)
      printf("i2c chip found at: %x, val = %d\n", port, res);
  }
}


