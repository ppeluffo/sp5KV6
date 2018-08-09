/*
 * sp5K_i2c.h
 *
 *  Created on: 18/10/2015
 *      Author: pablo
 */

// --------------------------------------------------------------------------------
// SPV5 DRIVER
// --------------------------------------------------------------------------------

#ifndef SRC_SP5KDRIVERS_DRV_I2C_SP5K_H_
#define SRC_SP5KDRIVERS_DRV_I2C_SP5K_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/twi.h>
#include "FreeRTOS.h"
#include "task.h"

#define SCL		0
#define SDA		1
#define I2C_MAXTRIES	5

#define ACK 	0
#define NACK 	1

void drv_I2C_init(void);
int drv_I2C_master_write ( const uint8_t slaveAddress, const uint8_t dataLength, const uint16_t dataAddress, char *pvBuffer, size_t xBytes );
int drv_I2C_master_read  ( const uint8_t slaveAddress, const uint8_t dataLength, const uint16_t dataAddress, char *pvBuffer, size_t xBytes );

//#define DEBUG_I2C

#endif /* SRC_SP5KDRIVERS_DRV_I2C_SP5K_H_ */
