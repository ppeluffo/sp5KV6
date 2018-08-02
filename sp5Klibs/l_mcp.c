/*
 *  sp5KFRTOS_mcp.c
 *
 *  Created on: 01/11/2013
 *      Author: root
 *
 *  Funciones del MCP23008 modificadas para usarse con FRTOS.
 */

#include "l_mcp.h"

// Funciones privadas del modulo MCP
static void pvMCP_init_MCP0(void);
static void pvMCP_init_MCP1(void);
//
//------------------------------------------------------------------------------------
// Funciones de uso general
//------------------------------------------------------------------------------------
void MCP_init(uint8_t device_id )
{
	// inicializo los MCP para la configuracion de pines del HW sp5K.
	// Como accedo al bus I2C, debo hacerlo una vez que arranco el RTOS.

	// Inicializo los pines del micro como entradas para las interrupciones del MCP.
	cbi(MCP0_DDR, MCP0_BIT);
	cbi(MCP1_DDR, MCP1_BIT);

	switch(device_id) {
	case 0:
		pvMCP_init_MCP0();
		break;
	case 1:
		pvMCP_init_MCP1();
		break;
	}

}
//------------------------------------------------------------------------------------
int MCP_modify( uint8_t dev_id, uint8_t dataAddress, uint8_t value, uint8_t bitMask )
{

uint8_t xBytes = 0;
uint8_t bus_address;
uint8_t	data_address_length = 1;
uint16_t data_address;
uint8_t i2c_error_code;
uint8_t regValue;
int xReturn = -1;

//	snprintf_P( debug_printfBuff,sizeof(debug_printfBuff),PSTR("MCPT&S [dev=%d][addr=%d][val=%d][msk=%d].\r\n\0"),deviceId, byteAddr, value,bitMask );
//	FreeRTOS_write( &pdUART1, debug_printfBuff, sizeof(debug_printfBuff) );

	// CONTROLO QUE SI EL MCP ES 23018 ESTE BIEN CONFIGURADO ANTES

	// Lo primero es obtener el semaforo
	frtos_ioctl( fdI2C,ioctl_OBTAIN_BUS_SEMPH, NULL);

	// Luego indicamos el periferico i2c en el cual queremos leer
	bus_address = MCP_id2busaddr(dev_id);
	frtos_ioctl(fdI2C,ioctl_I2C_SET_SLAVE_ADDRESS, &bus_address);

	// Luego indicamos cuantos bytes queremos leer del dispositivo: largo.
	// En los MCP se lee y escribe de a 1 registro.
	data_address_length = 1;
	frtos_ioctl(fdI2C,ioctl_I2C_SET_DATA_LENGTH, &data_address_length);

	// Ahora indicamos desde que posicion queremos leer: direccion
	data_address = (uint16_t)(dataAddress);
	frtos_ioctl(fdI2C,ioctl_I2C_SET_DATA_ADDRESS,&data_address);

	// Por ultimo leemos.
	xBytes = 1;
	xReturn = frtos_read(fdI2C, &regValue, xBytes);

	i2c_error_code = frtos_ioctl(fdI2C, ioctl_I2C_GET_LAST_ERROR, NULL );
	if (i2c_error_code != I2C_OK ) {
		xprintf_P(PSTR("ERROR: I2C RD err(%d).\r\n\0"),i2c_error_code );
	}

	if (xReturn != xBytes ) {
		xReturn = -1;
		goto quit;
	}

//	snprintf_P( debug_printfBuff,sizeof(debug_printfBuff),PSTR("MCPT&S [rdVal=%d]\r\n\0"),regValue );
//	FreeRTOS_write( &pdUART1, debug_printfBuff, sizeof(debug_printfBuff) );

	// Modifico el registro
/*	if  (regValue == 0) {
		regValue &= ~BV(bitMask);
	} else {
		regValue |= BV(bitMask);
	}
*/
	// Escribo en el MCP
	xBytes = 1;
	xReturn = frtos_write(fdI2C, &regValue, xBytes);
	i2c_error_code = frtos_ioctl(fdI2C, ioctl_I2C_GET_LAST_ERROR, NULL );
	if (i2c_error_code != I2C_OK ) {
		xprintf_P(PSTR("ERROR: MCP WR err (%d).\r\n\0"), i2c_error_code );
	}

	if (xReturn != xBytes ) {
		xReturn = -1;
		goto quit;
	}

quit:

	frtos_ioctl(fdI2C,ioctl_RELEASE_BUS_SEMPH, NULL);
	return(xReturn);
}
//------------------------------------------------------------------------------------
// FUNCIONES PRIVADAS
//------------------------------------------------------------------------------------
uint8_t MCP_id2busaddr( uint8_t id )
{
	switch(id) {
	case 0:
		return(MCP0_ADDR);
		break;
	case 1:
		return(MCP1_ADDR);
		break;
	default:
		return(99);
		break;

	}

	return(99);

}
//------------------------------------------------------------------------------------
static void pvMCP_init_MCP0(void)
{
	// inicializa el MCP23008 de la placa de logica
	// NO CONTROLO ERRORES.

uint8_t data;

	// MCP0_IODIR: inputs(1)/outputs(0)
	data = 0;
	data |= ( BV(MCP0_GPIO_IGPRSDCD) | BV(MCP0_GPIO_IGPRSRI) );
	MCP_write( 0, MCP0_IODIR, &data, 1 );

	// MCP0_IPOL: polaridad normal
	data = 0;
	MCP_write( 0, MCP0_IPOL, &data, 1 );

	// MCP0_GPINTEN: inputs interrupt on change.
	data = 0;
	//data |= ( BV(MCP_GPIO_IGPRSDCD) | BV(MCP_GPIO_IGPRSRI) | BV(MCP_GPIO_ITERMPWRSW) );
	data |=  BV(MCP0_GPIO_IGPRSDCD);
	MCP_write( 0, MCP0_GPINTEN, &data, 1 );

	// MCP0_INTCON: Compara contra su valor anterior
	data = 0;
	MCP_write( 0, MCP0_INTCON, &data, 1 );

	// MCP0_IOCON: INT active H
	data = 2;
	MCP_write( 0, MCP0_IOCON, &data, 1 );

	// MCP0_GPPU: pull-ups
	// Habilito los pull-ups en DCD
	//data = 0;
	//MCP_write( 0, MCP0_GPPU, data, 1 );

	// TERMPWR ON
	// Al arrancar prendo la terminal para los logs iniciales.
	data = 0;
	data |= BV(MCP0_GPIO_OTERMPWR);	// TERMPWR = 1
	MCP_write( 0, MCP0_OLAT, &data, 1 );

	xprintf_P(PSTR("MCP0 init OK\r\n\0"));
}
//------------------------------------------------------------------------------------
static void pvMCP_init_MCP1(void)
{
	// Inicializo el MCP23018 de la placa analogica
	// El modo 0 indica uso normal, el modo 1 uso por reconfiguracion por lo que el log es por D_DEBUG

uint8_t data;

	// IOCON
	data = 0x63; // 0110 0011
	//                      1->INTCC:Read INTCAP clear interrupt
	//                     1-->INTPOL: INT out pin active high
	//                    0--->ORDR: Active driver output. INTPOL set the polarity
	//                   0---->X
	//                 0----->X
	//                1------>SEQOP: sequential disabled. Address ptr does not increment
	//               1------->MIRROR: INT pins are ored
	//              0-------->BANK: registers are in the same bank, address sequential
	MCP_write( 1, MCP1_IOCON, data, 1 );

	// DIRECCION
	// 0->output
	// 1->input
	data = 0x80; // 1000 0000 ( GPA0..GPA6: outputs, GPA7 input )
	MCP_write( 1, MCP1_IODIRA, data, 1 );

	data = 0x64; // 0110 0100
	MCP_write( 1, MCP1_IODIRB, data, 1 );
	//
	// PULL-UPS
	// 0->disabled
	// 1->enabled
	// Los dejo en 0 para ahorrar ma.
	//data = 0x00; // 1111 1111
	data = 0xF8; // 1111 1000
	MCP_write( 1, MCP1_GPPUA, data, 1 );

	data = 0xF3; // 1111 0000
	MCP_write( 1, MCP1_GPPUB, data, 1 );
	//
	// Valores iniciales de las salidas en 0
	data = 0x00;
	MCP_write( 1,  MCP1_OLATA, data, 1 );

	data = 0x00;
	MCP_write( 1, MCP1_OLATB, data, 1 );
	//
	// GPINTEN: inputs interrupt on change.
	// Habilito que DIN0/1 generen una interrupcion on-change.
	// El portA no genera interrupciones
//	val = MCP1_GPINTENA;
//	FreeRTOS_ioctl(&pdI2C,ioctl_I2C_SET_BYTEADDRESS,&val, DEBUG_MCP);
//	data = 0;
//	xBytes = sizeof(data);
//	xReturn = FreeRTOS_write(&pdI2C, &data, xBytes);
	//data = 0x60; // 0110 0000
	//data |= ( BV(MCP1_GPIO_DIN0) | BV(MCP1_GPIO_DIN1) );
//	val = MCP1_GPINTENB;
//	FreeRTOS_ioctl(&pdI2C,ioctl_I2C_SET_BYTEADDRESS,&val, DEBUG_MCP);
//	data = 0;
//	xBytes = sizeof(data);
//	xReturn = FreeRTOS_write(&pdI2C, &data, xBytes);
	//
	// DEFVALB: valores por defecto para comparar e interrumpir
	//data = 0;
	//status = pvMCP_write( MCP1_DEFVALB, MCP_ADDR2, 1, &data);

	// INTCON: controlo como comparo para generar la interrupcion.
	// Con 1, comparo contra el valor fijado en DEFVAL
	// Con 0#ifdef SP5KV5_3CH vs. su valor anterior.
//	val = MCP1_INTCONB;
//	FreeRTOS_ioctl(&pdI2C,ioctl_I2C_SET_BYTEADDRESS,&val, DEBUG_MCP);
	//data |= ( BV(MCP1_GPIO_DIN0) | BV(MCP1_GPIO_DIN1) );
//	data = 0;
//	xBytes = sizeof(data);
//	xReturn = FreeRTOS_write(&pdI2C, &data, xBytes);
	// Borro interrupciones pendientes
//	val = MCP1_INTCAPB;
//	FreeRTOS_ioctl(&pdI2C,ioctl_I2C_SET_BYTEADDRESS,&val, DEBUG_MCP);
//	data = 0;
//	xBytes = sizeof(data);
//	xReturn = FreeRTOS_write(&pdI2C, &data, xBytes);

}
//------------------------------------------------------------------------------------
