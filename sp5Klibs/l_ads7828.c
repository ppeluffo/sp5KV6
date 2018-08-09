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
int8_t xBytes = 0;
uint8_t bus_address;
uint8_t	data_address_length = 1;
uint16_t data_address;
uint8_t i2c_error_code;


	if ( adc_channel > 7) {
		goto quit;
	}

	// Convierto el canal 0-7 al C2/C1/C0 requerido por el conversor.
	ads7828Channel = (((adc_channel>>1) | (adc_channel&0x01)<<2)<<4) | ADS7828_CMD_SD;

	// do conversion
	// Armo el COMMAND BYTE
	ads7828CmdByte = ads7828Channel & 0xF0;	// SD=1 ( single end inputs )
	ads7828CmdByte |= ADS7828_CMD_PDMODE2;	// Internal reference ON, A/D converter ON

	// start conversion on requested channel
	// Lo primero es obtener el semaforo
	frtos_ioctl( fdI2C,ioctl_OBTAIN_BUS_SEMPH, NULL);

	// 1) Indicamos el periferico i2c en el cual queremos leer ( variable de 8 bits !!! )
	bus_address = ADS7828_ADDR;
	frtos_ioctl(fdI2C,ioctl_I2C_SET_SLAVE_ADDRESS, &bus_address);

	// 2) Luego indicamos la direccion desde donde leer:
	//    Largo: 1 byte indica el largo. El FRTOS espera 1 byte.
	data_address_length = 1;
	frtos_ioctl(fdI2C,ioctl_I2C_SET_DATA_LENGTH, &data_address_length);
	// 	Direccion: El FRTOS espera siempre 2 bytes.
	data_address = 0;
	frtos_ioctl(fdI2C,ioctl_I2C_SET_DATA_ADDRESS,&data_address);

	// Escribo en el ADS el comando.
	xBytes = 1;
	xReturn = frtos_write(fdI2C, (char *)&ads7828CmdByte, xBytes);
	i2c_error_code = frtos_ioctl(fdI2C, ioctl_I2C_GET_LAST_ERROR, NULL );
	if (i2c_error_code != I2C_OK ) {
		xprintf_P(PSTR("ERROR: ADC WR err (%d).\r\n\0"), i2c_error_code );
	}

	if (xReturn != xBytes ) {
		xReturn = -1;
	}

	// El comando prendio el ADC por lo que espero el settle time
	vTaskDelay(1);

	// Leo el resultado de la conversion ( 2 bytes )
	xBytes = 2;
	xReturn = frtos_read(fdI2C, (char *)&buffer, xBytes);

	if (xReturn != xBytes ) {
		xReturn = -1;
	}

	xReturn = (buffer[0]<<8) | buffer[1];

	// Apago el conversor
	//	ads7828CmdByte = ads7828Channel & 0xF0;
	//	ads7828CmdByte |= ADS7828_CMD_PDMODE0;	// Internal reference OFF, A/D converter OFF
	//	status = pvADS7828_write( &ads7828CmdByte);

quit:

	// Y libero el semaforo.
	frtos_ioctl( fdI2C,ioctl_RELEASE_BUS_SEMPH, NULL);
	return(xReturn);

}

// ---------------------------------------------------------------------------

