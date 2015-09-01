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

/* Si7021 CMD Code */
#define Measure_RH_M      0xE5
#define Measure_RH_NM     0xF5
#define Measure_T_M       0xE3
#define Measure_T_M       0xF3
#define Read_Temp_RH      0xE0
#define RESET             0xFE
#define Write_RH_T_REG    0xE6
#define Read_RH_T_REG     0xE7
#define Read_ID_1_1       0xFA
#define Read_ID_1_2       0x0F
#define Read_ID_2_1       0xFC
#define Read_ID_2_2       0xC9
#define Read_Rev_1        0x84
#define Read_Rev_2        0xB8

/* ID Register */
#define ID_SI7021         0x15

#define WAKE_UP_TIME      15
#define SI7021_ADR        0x40

/* Coefficients */
#define TEMPERATURE_OFFSET   46.85
#define TEMPERATURE_MULTIPLE 175.72
#define TEMPERATURE_SLOPE    65536
#define HUMIDITY_OFFSET      6
#define HUMIDITY_MULTIPLE    125
#define HUMIDITY_SLOPE       65536

int fd;
float _last_temperature = 0.0;

//uint8_t register_rw(const uint8_t reg_addr, uint16_t *word);
//uint8_t register_rb(const uint8_t reg_addr, uint8_t *byte);
//uint8_t register_wb(const uint8_t reg_addr, uint8_t byte);
unsigned int _tempMeasurement(void);
unsigned int _RHMeasurement(void);
float getTemperature(void);
float getHumidity(void);

//-----------------------------------------------------------------------------------------

///** Register Read (word).
// *
// * @param reg_addr the register address.
// * @param byte the data to be read.
// */
//uint8_t register_rw(const uint8_t reg_addr, uint16_t *word)
//{	
//	*word =  i2c_smbus_read_word_data(fd,reg_addr);
//	//printf("Word : %d \n",*word);
//	
//	return 0;
//}
//
///** Register Read (Byte).
// *
// * @param reg_addr the register address.
// * @param byte the data to be read.
// */
//uint8_t register_rb(const uint8_t reg_addr, uint8_t *byte)
//{
//	*byte =  i2c_smbus_read_byte_data(fd,reg_addr);
//	//printf("Byte : %d \n",*byte);
//	
//	return 0;
//}
//
///** Register write (Byte).
// *
// * @param reg_addr the register address.
// * @param byte the data to be written.
// */
//uint8_t register_wb(const uint8_t reg_addr, uint8_t byte)
//{
//	uint8_t error;
//
//	error = i2c_smbus_write_byte_data(fd, reg_addr, byte);
//	return (error);
//}
//
//void _delay_ms(unsigned ms) 
//{
//	lib_delay_us(ms*1000);
//}


////------------------------------------------------------------
//// calibrated with good accuracy for GESBC-9G20u board
//void lib_delay_us(unsigned long us) 
//{
//	volatile unsigned long i = 79 * us / 2;
//	while(i --);
//}

// Temperature measurement
unsigned int _tempMeasurement()
{
  unsigned int rawData;
  uint8_t err;
  uint8_t word_MSB;
  uint8_t word_LSB;
  
  _delay_ms(10); //delay(10);
  //Wire.beginTransmission( SI7021_ADR );
  err = register_wb(SI7021_ADR, 0xE3);	//Wire.write(0xE3);
  //Wire.endTransmission( false );							
//  Wire.requestFrom( SI7021_ADR, 2);
//  while ( Wire.available( ) < 2 )					
//  {;;}
  err = register_rb(SI7021_ADR, &word_MSB);
  err = register_rb(SI7021_ADR, &word_LSB);
  
//  rawData = ( Wire.read() << 8 );					
//  rawData |= Wire.read( );
  rawData = word_MSB;
  rawData = rawData << 8;
  rawData = rawData + word_LSB;
  //Wire.endTransmission( );	
  return rawData;
}

float getTemperature( )
{
  float rawTemperature;

  rawTemperature = _tempMeasurement();
  _last_temperature = ( rawTemperature * TEMPERATURE_MULTIPLE) / TEMPERATURE_SLOPE- TEMPERATURE_OFFSET;

  return _last_temperature;
}

// Humidity measurement
unsigned int _RHMeasurement()
{
  unsigned int rawData;
  uint8_t err;
  uint8_t word_MSB;
  uint8_t word_LSB;
    
  _delay_ms(10);	//delay(10);
  //Wire.beginTransmission( SI7021_ADR );
  //Wire.write(0xE5);
  //Wire.endTransmission( false );
  err = register_wb(SI7021_ADR, 0xE5);
  
//  Wire.requestFrom( SI7021_ADR, 2);
//  while ( Wire.available( ) < 2 )					
//  {;;}
  err = register_rb(SI7021_ADR, &word_MSB);
  err = register_rb(SI7021_ADR, &word_LSB);
  
//  rawData = ( Wire.read() << 8 );					
//  rawData |= Wire.read( );							
//  Wire.endTransmission( );	
  rawData = word_MSB;
  rawData = rawData << 8;
  rawData = rawData + word_LSB;
  
  return rawData;
}

float getHumidity()
{
  float rawHumidity, curve, linearHumidity;

  rawHumidity = _RHMeasurement( );
  linearHumidity = (rawHumidity * HUMIDITY_MULTIPLE) / HUMIDITY_SLOPE - HUMIDITY_OFFSET;

  return linearHumidity;
}
//-------------------------------------------------------------------------------------------

//// Search the I2C address space for slaves.
//void scan_i2c_bus(int file)
//{
//  int port;
//  
//  for (port=0; port<127; port++)
//  {
//    int res;
//    if (ioctl(file, I2C_SLAVE, port) < 0)
//      perror("ioctl() I2C_SLAVE failed\n");
//    res = i2c_smbus_read_byte(file);
//    if (res >= 0)
//      printf("i2c chip found at: %x, val = %d\n", port, res);
//  }
//}


