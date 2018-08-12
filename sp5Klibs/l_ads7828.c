/*
 * l_ads7828.c
 *
 *  Created on: 9 ago. 2018
 *      Author: pablo
 */

#include "l_ads7828.h"

/* EL ADS7828 si bien es I2C requiere un timing especial por lo cual no
 * puedo usar el I2C read / write sino que debo implementar la funcion completa
 */

int16_t ADC_read( uint8_t adc_channel )
{
	// Lee un canal 0..7 del ADC.
	// Retorna su valor (12bits) o -1 si hubo error.

uint8_t ads7828Channel;
uint8_t ads7828CmdByte;
uint8_t buffer[2];
int16_t xReturn = -1;

	if ( adc_channel > 7) {
		goto quit;
	}

	// Convierto el canal 0-7 al C2/C1/C0 requerido por el conversor.
	ads7828Channel = (((adc_channel>>1) | (adc_channel&0x01)<<2)<<4) | ADS7828_CMD_SD;

	// do conversion
	// Armo el COMMAND BYTE
	ads7828CmdByte = ads7828Channel & 0xF0;	// SD=1 ( single end inputs )
	ads7828CmdByte |= ADS7828_CMD_PDMODE2;	// Internal reference ON, A/D converter ON

	xReturn = I2C_read(ADS7828_ADDR, ads7828CmdByte, (char *)&buffer, 2);
	if (xReturn != 2 ) {
	//	xprintf_P(PSTR("ADC error\r\n\0"));
		xReturn = -1;
	}

	xReturn = (buffer[0]<<8) | buffer[1];

	// Apago el conversor
	//	ads7828CmdByte = ads7828Channel & 0xF0;
	//	ads7828CmdByte |= ADS7828_CMD_PDMODE0;	// Internal reference OFF, A/D converter OFF
	//	status = pvADS7828_write( &ads7828CmdByte);

quit:

	return(xReturn);

}

// ---------------------------------------------------------------------------

