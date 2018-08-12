/*
 * sp5K_i2c.c
 *
 *  Created on: 18/10/2015
 *      Author: pablo
 */

// --------------------------------------------------------------------------------
// SPV5 DRIVER
// --------------------------------------------------------------------------------

#include "drv_i2c_sp5K.h"
#ifdef DEBUG_I2C
#include "l_printf.h"
#endif

static void pvI2C_setBitRate(int bitrateKHz);
static void pvI2C_sendStart(void);
static void pvI2C_sendStop(void);
static void pvI2C_sendByte(uint8_t data);
static void pvI2C_readByte(uint8_t ackFlag);
static uint8_t pvI2C_getStatus(void);
static  bool pvI2C_waitForComplete(void);
static void pvI2C_disable(void);

//------------------------------------------------------------------------------------
void drv_I2C_init(void)
{
	// set pull-up resistors on I2C bus pins
	//sbi(PORTC, 0);  // i2c SCL
	//sbi(PORTC, 1);  // i2c SDA
	PORTC |= 1 << SCL;
	PORTC |= 1 << SDA;
	// set i2c bit rate to 100KHz
	pvI2C_setBitRate(100);
}
//------------------------------------------------------------------------------------
int drv_I2C_master_write ( const uint8_t slaveAddress, const uint8_t dataLength, const uint16_t dataAddress, char *pvBuffer, size_t xBytes  )
{

uint8_t tryes = 0;
uint8_t i2c_status;
char txbyte;
uint8_t i;
int xReturn = -1;

i2c_retry:

#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MW0: 0x%02x,0x%02x,0x%02x,0x%02x\r\n\0"),slaveAddress,dataLength, (uint16_t)(dataAddress), xBytes );
#endif

	if (tryes++ >= I2C_MAXTRIES) goto i2c_quit;

	// Pass1) START: Debe retornar 0x08 (I2C_START) o 0x10 (I2C_REP_START) en condiciones normales
	pvI2C_sendStart();
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MW1: 0x%02x\r\n\0"),i2c_status );
#endif
	if ( i2c_status == TW_MT_ARB_LOST) goto i2c_retry;
	if ( (i2c_status != TW_START) && (i2c_status != TW_REP_START) ) goto i2c_quit;

	// Pass2) (SLA_W) Send slave address. Debo recibir 0x18 ( SLA_ACK )
	txbyte = slaveAddress | TW_WRITE;
	pvI2C_sendByte(txbyte);
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MW2: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );

#endif
	// Check the TWSR status
	if ((i2c_status == TW_MT_SLA_NACK) || (i2c_status == TW_MT_ARB_LOST)) goto i2c_retry;
	if (i2c_status != TW_MT_SLA_ACK) goto i2c_quit;

	// Pass3) Envio la direccion fisica donde comenzar a escribir.
	// En las memorias es una direccion de 2 bytes.En el DS1344 o el BusController es de 1 byte
	// Envio primero el High 8 bit i2c address. Debo recibir 0x28 ( DATA_ACK)
	if ( dataLength == 2 ) {
		txbyte = (dataAddress) >> 8;
		pvI2C_sendByte(txbyte);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MW3H: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
		if (i2c_status != TW_MT_DATA_ACK) goto i2c_quit;
	}

	// Envio el Low 8 byte i2c address.
	if ( dataLength >= 1 ) {
		txbyte = (dataAddress) & 0x00FF;
		pvI2C_sendByte(txbyte);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MW3L: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
		if (i2c_status != TW_MT_DATA_ACK) goto i2c_quit;
	}

	// Pass4) Trasmito todos los bytes del buffer. Debo recibir 0x28 (DATA_ACK)
	for ( i=0; i < xBytes; i++ ) {
		txbyte = *pvBuffer++;
		pvI2C_sendByte(txbyte);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MW4: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
		if (i2c_status != TW_MT_DATA_ACK) goto i2c_quit;
	}

	xReturn = xBytes;

i2c_quit:

	// Pass5) STOP
	pvI2C_sendStop();
	vTaskDelay( ( TickType_t)( 20 / portTICK_RATE_MS ) );

	// En caso de error libero la interface.
	if (xReturn == -1)
		pvI2C_disable();

#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MW5 xReturn: 0x%02x\r\n\0"),xReturn );
#endif

	return(xReturn);

}
//------------------------------------------------------------------------------------
int drv_I2C_master_read  ( const uint8_t slaveAddress, const uint8_t dataLength, const uint16_t dataAddress, char *pvBuffer, size_t xBytes  )
{
	// En el caso del ADC, el read no lleva la parte de mandar la SLA+W. !!!!!

uint8_t tryes = 0;
uint8_t i2c_status;
char txbyte;
int xReturn = -1;
uint8_t i;

i2c_retry:

#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR0: 0x%02x,0x%02x,0x%02x,0x%02x\r\n\0"),slaveAddress,dataLength, (uint16_t)(dataAddress), xBytes );
#endif

	if (tryes++ >= I2C_MAXTRIES) goto i2c_quit;

	// Pass1) START: Debe retornar 0x08 (I2C_START) o 0x10 (I2C_REP_START) en condiciones normales
	pvI2C_sendStart();
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR1: 0x%02x\r\n\0"),i2c_status );
#endif
	if ( i2c_status == TW_MT_ARB_LOST) goto i2c_retry;
	if ( (i2c_status != TW_START) && (i2c_status != TW_REP_START) ) goto i2c_quit;

	// Pass2) (SLA_W) Send slave address. Debo recibir 0x18 ( SLA_ACK )
	txbyte = slaveAddress | TW_WRITE;
	pvI2C_sendByte(txbyte);
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	// Check the TWSR status
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR2: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
	if ((i2c_status == TW_MT_SLA_NACK) || (i2c_status == TW_MT_ARB_LOST)) goto i2c_retry;
	if (i2c_status != TW_MT_SLA_ACK) goto i2c_quit;

	// Pass3) Envio la direccion fisica donde comenzar a escribir.
	// En las memorias es una direccion de 2 bytes.En el DS1344 o el BusController es de 1 byte
	// Envio primero el High 8 bit i2c address. Debo recibir 0x28 ( DATA_ACK)
	if ( dataLength == 2 ) {
		txbyte = (dataAddress) >> 8;
		pvI2C_sendByte(txbyte);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MR3H: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
		if (i2c_status != TW_MT_DATA_ACK) goto i2c_quit;
	}

	// Envio el Low 8 byte i2c address.
	if ( dataLength >= 1 ) {
		txbyte = (dataAddress) & 0x00FF;
		pvI2C_sendByte(txbyte);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MR3L: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
		if (i2c_status != TW_MT_DATA_ACK) goto i2c_quit;
	}

	// Pass4) REPEATED START. Debo recibir 0x10 ( REPEATED_START)
	pvI2C_sendStart();
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR4: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
	if (i2c_status == TW_MT_ARB_LOST) goto i2c_retry;
	if ( (i2c_status != TW_START) && (i2c_status != TW_REP_START) ) goto i2c_quit;

	// Pass5) (SLA_R) Send slave address + READ. Debo recibir un 0x40 ( SLA_R ACK)
	txbyte = slaveAddress | TW_READ;
	pvI2C_sendByte(txbyte);
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	// Check the TWSR status
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR5: 0x%02x,0x%02x\r\n\0"),txbyte,i2c_status );
#endif
	if ((i2c_status == TW_MR_SLA_NACK) || (i2c_status == TW_MR_ARB_LOST)) goto i2c_retry;
	if (i2c_status != TW_MR_SLA_ACK) goto i2c_quit;

	// Pass6) Leo todos los bytes requeridos y respondo con ACK.
	for ( i=0; i < (xBytes-1); i++ ) {
		pvI2C_readByte(ACK);
		if ( !pvI2C_waitForComplete() )
			goto i2c_quit;
		i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
		xprintf_P( PSTR("I2C_MR6(%d): 0x%02x,0x%02x\r\n\0"),i,TWDR,i2c_status );
#endif
		if (i2c_status != TW_MR_DATA_ACK) goto i2c_quit;
		*pvBuffer++ = TWDR;
	}

	// Pass7) Acepto el ultimo byte y respondo con NACK
	pvI2C_readByte(NACK);
	if ( !pvI2C_waitForComplete() )
		goto i2c_quit;
	i2c_status = pvI2C_getStatus();
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR7(%d): 0x%02x,0x%02x\r\n\0"),i,TWDR,i2c_status );
#endif
	if (i2c_status != TW_MR_DATA_NACK) goto i2c_quit;
	*pvBuffer++ = TWDR;

	*pvBuffer = '\0';

	// I2C read OK.
	xReturn = xBytes;

i2c_quit:

	// Pass5) STOP
	pvI2C_sendStop();

	// En caso de error libero la interface.
	if (xReturn == -1)
		pvI2C_disable();

//	if ( retV ) {
//		snprintf_P( debug_printfBuff,CHAR128,PSTR("I2C_MR RET TRUE ( %d )\r\n\0"),retV );
//	} else {
//		snprintf_P( debug_printfBuff,CHAR128,PSTR("I2C_MR RET FALSE ( %d )\r\n\0"),retV );
//	}

//	FreeRTOS_write( &pdUART1, debug_printfBuff, sizeof(debug_printfBuff) );
#ifdef DEBUG_I2C
	xprintf_P( PSTR("I2C_MR8 xReturn: 0x%02x\r\n\0"),xReturn );
#endif

	return(xReturn);

}
//------------------------------------------------------------------------------------
// FUNCIONES AUXILIARES PRIVADAS DE I2C
//------------------------------------------------------------------------------------
static void pvI2C_setBitRate(int bitrateKHz)
{
int bitrate_div;

	// SCL freq = F_CPU/(16+2*TWBR*4^TWPS)

	// set TWPS to zero
	//cbi(TWSR, TWPS0);
	//cbi(TWSR, TWPS1);
	((TWSR) &= ~(1 << (TWPS0)));
	((TWSR) &= ~(1 << (TWPS1)));
	// SCL freq = F_CPU/(16+2*TWBR))

	// calculate bitrate division
	bitrate_div = ((F_CPU/1000l)/bitrateKHz);
	if(bitrate_div >= 16) {
		bitrate_div = (bitrate_div-16)/2;
	}

	TWBR = bitrate_div;
}
//------------------------------------------------------------------------------------
static void pvI2C_sendStart(void)
{
	// Genera la condicion de START en el bus I2C.
	// TWCR.TWEN = 1 : Habilita la interface TWI
	// TWCR.TWSTA = 1: Genera un START
	// TWCR.TWINT = 1: Borra la flag e inicia la operacion del TWI
	//
	// Cuando el bus termino el START, prende la flag TWCR.TWINT y el codigo
	// de status debe ser 0x08 o 0x10.
	//

	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

}
//-----------------------------------------------------------------------------------
static void pvI2C_sendStop(void)
{
	// Genera la condicion STOP en el bus I2C.
	// !!! El TWINT NO ES SETEADO LUEGO DE UN STOP
	// por lo tanto no debo esperar que termine.

	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

}
//------------------------------------------------------------------------------------
static void pvI2C_sendByte( uint8_t data)
{

	// Envia el data por el bus I2C.
	TWDR = data;
	// Habilita la trasmision y el envio de un ACK.
	TWCR = (1 << TWINT) | (1 << TWEN);

}
//------------------------------------------------------------------------------------
static void pvI2C_readByte( uint8_t ackFlag)
{
	// begin receive over i2c
	if( ackFlag == ACK)
	{
		// ackFlag = TRUE: ACK the recevied data
		TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
	}
	else
	{
		// ackFlag = FALSE: NACK the recevied data
		TWCR = (1 << TWINT) | (1 << TWEN);
	}

}
//------------------------------------------------------------------------------------
static uint8_t pvI2C_getStatus(void)
{
	// retieve current i2c status from i2c TWSR
	return( TWSR & 0xF8 );
}
//------------------------------------------------------------------------------------
static bool pvI2C_waitForComplete(void)
{
TickType_t xTicksToWait = 3;		// 3 ticks ( 30ms es el maximo tiempo que espero )
TimeOut_t xTimeOut;

	// inicializo el timeout
	vTaskSetTimeOutState( &xTimeOut );

	// wait for i2c interface to complete operation
	while( !(TWCR & (1<<TWINT)) )
	{
		if( xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) != pdFALSE ) {
			// Timed out
			return(false);
		}
	}

	return(true);
}
//------------------------------------------------------------------------------------
static void pvI2C_disable(void)
{
	// Deshabilito la interfase TWI
	TWCR &= ~(1 << TWEN);

}
//------------------------------------------------------------------------------------
