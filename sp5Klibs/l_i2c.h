/*
 * l_i2c.h
 *
 *  Created on: 26 de mar. de 2018
 *      Author: pablo
 */

#ifndef SPX_LIBS_L_I2C_H_
#define SPX_LIBS_L_I2C_H_

#include "frtos_io.h"
#include "l_printf.h"

#define BUSADDR_EEPROM_M2402	0xA0
#define BUSADDR_RTC_DS1340		0xD0
#define MCP0_ADDR				0x40	// MCP23008
#define MCP1_ADDR				0x4E	// MCP23018
#define ADS7828_ADDR			0x90

int8_t I2C_read( uint8_t i2c_slave_address, uint32_t rdAddress, char *data, uint8_t length );
int8_t I2C_write( uint8_t i2c_slave_address, uint32_t wrAddress, char *data, uint8_t length );

#endif /* SPX_LIBS_L_I2C_H_ */
