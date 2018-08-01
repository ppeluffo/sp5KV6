/*
 * drv_uart_spx.h
 *
 *  Created on: 11 jul. 2018
 *      Author: pablo
 *
 * Funcionamiento
 * Las UARTs solo dependen del driver.
 * Tenemos por un lado una uart fisica ( PORTD en el caso del USB ).
 * Sobre esta implementamos las rutinas ISR.
 * Para manejarlas, creamos una estrucutura que tenga un identificador del puerto que llamamos uart_id_t
 * que nos sirve para saber en que puerto debemos actuar.
 * Tambien tiene 2 ringBuffers, TX y RX.
 * Las uarts son estaticas, propiedad del driver.
 * Al inicializarlas, la funcion drv_uart_init devuelve un puntero a la estructura de la uart, para que
 * el frtos pueda asociarla a un periferico.
 *
 */


#ifndef SRC_DRIVERS_DRV_UART_SP5K_H_
#define SRC_DRIVERS_DRV_UART_SP5K_H_

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"

#include "l_ringBuffer.h"

//-----------------------------------------------------------------------
/* Constants for writing to UCSRB. */
#define serRX_INT_ENABLE				( ( unsigned portCHAR ) 0x80 )
#define serRX_ENABLE					( ( unsigned portCHAR ) 0x10 )
#define serTX_INT_ENABLE				( ( unsigned portCHAR ) 0x20 )
#define serTX_ENABLE					( ( unsigned portCHAR ) 0x08 )

/* Constants for writing to UCSRC. */
#define serUCSRC_SELECT					( ( unsigned portCHAR ) 0x00 )
#define serEIGHT_DATA_BITS				( ( unsigned portCHAR ) 0x06 )

#define UARTCTL_PORT	PORTD
#define UARTCTL_PIN		PIND
#define UARTCTL			4
#define UARTCTL_DDR		DDRD
#define UARTCTL_MASK	0x10

//-----------------------------------------------------------------------
#define RS232_RXSTORAGE_SIZE	128
#define RS232_TXSTORAGE_SIZE	128
uint8_t rs232_rxStorage[RS232_RXSTORAGE_SIZE];
uint8_t rs232_txStorage[RS232_TXSTORAGE_SIZE];

#define GPRS_RXSTORAGE_SIZE	512
#define GPRS_TXSTORAGE_SIZE	128
uint8_t gprs_rxStorage[GPRS_RXSTORAGE_SIZE];
uint8_t gprs_txStorage[GPRS_TXSTORAGE_SIZE];

// Enumenerador de los puertos fisicos.
typedef enum {
	iUART_RS232 = 0,
	iUART_GPRS,
} uart_id_t;

// Estructura generica de una UART
typedef struct {
	uart_id_t uart_id;	// Identificador de la uart fisico
	ringBuffer_s TXringBuffer;	// ringbuffer de trasmision
	ringBuffer_s RXringBuffer;	// ringbuffer de recepcion.
} uart_control_t;

// Creo las uart's en memoria.
uart_control_t uart_rs232, uart_gprs;

//-----------------------------------------------------------------------
uart_control_t *drv_uart_init( uart_id_t iUART, uint32_t baudrate );
void drv_uart_interruptOn(uart_id_t iUART);
void drv_uart_interruptOff(uart_id_t iUART);
void drv_set_baudrate(uint32_t baudRate, uint8_t *baudA, uint8_t *baudB, uint8_t *ctl );

void drv_uart_rs232_open( uint32_t baudrate );
void drv_uart_gprs_open( uint32_t baudrate );
//-----------------------------------------------------------------------

#endif /* SRC_DRIVERS_DRV_UART_S5K_H_ */
