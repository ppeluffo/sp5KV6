/*
 * drv_uart_spx.c
 *
 *  Created on: 11 jul. 2018
 *      Author: pablo
 */

#include "drv_uart_sp5K.h"

//----------------------------------------------------------------------------------------
uart_control_t *drv_uart_init( uart_id_t iUART, uint32_t baudrate )
{

uart_control_t *pUart = NULL;

	switch(iUART) {
	case iUART_RS232:
		// Abro el puerto serial y fijo su velocidad
		drv_uart_rs232_open(baudrate);
		// Inicializo los ringBuffers que manejan el puerto. Son locales al driver.
		rBufferCreateStatic( &uart_rs232.RXringBuffer, &rs232_rxStorage[0], RS232_RXSTORAGE_SIZE );
		rBufferCreateStatic( &uart_rs232.TXringBuffer, &rs232_txStorage[0], RS232_RXSTORAGE_SIZE );
		// Asigno el identificador
		uart_rs232.uart_id = iUART_RS232;
		// Devuelvo la direccion de uart_usb para que la asocie al dispositvo USB el frtos.
		pUart = (uart_control_t *)&uart_rs232;
		break;
	case iUART_GPRS:
		// Abro el puerto serial y fijo su velocidad
		drv_uart_gprs_open(baudrate);
		// Inicializo los ringBuffers que manejan el puerto. Son locales al driver.
		rBufferCreateStatic( &uart_gprs.RXringBuffer, &gprs_rxStorage[0], GPRS_RXSTORAGE_SIZE );
		rBufferCreateStatic( &uart_gprs.TXringBuffer, &gprs_txStorage[0], GPRS_RXSTORAGE_SIZE );
		// Asigno el identificador
		uart_gprs.uart_id = iUART_GPRS;
		// Devuelvo la direccion de uart_gprs para que la asocie al dispositvo GPRS el frtos.
		pUart = (uart_control_t *)&uart_gprs;
		break;
	}

	return(pUart);
}
//----------------------------------------------------------------------------------------
void drv_uart_interruptOn(uart_id_t iUART)
{
	// Habilito la interrupcion TX del UART lo que hace que se ejecute la ISR_TX y
	// esta vaya a la TXqueue y si hay datos los comienze a trasmitir.

uint8_t ucByte;

	switch(iUART) {
	case iUART_GPRS:
		ucByte = UCSR0B;
		ucByte |= serTX_INT_ENABLE;
		UCSR0B = ucByte;
		break;
	case iUART_RS232:
		ucByte = UCSR1B;
		ucByte |= serTX_INT_ENABLE;
		UCSR1B = ucByte;
		break;
	}

}
//----------------------------------------------------------------------------------------
void drv_uart_interruptOff(uart_id_t iUART)
{

uint8_t ucByte;

	switch(iUART) {
	case iUART_GPRS:
		ucByte = UCSR0B;
		ucByte &= ~serTX_INT_ENABLE;
		UCSR0B = ucByte;
		break;
	case iUART_RS232:
		ucByte = UCSR1B;
		ucByte &= ~serTX_INT_ENABLE;
		UCSR1B = ucByte;
		break;
	}

}
//----------------------------------------------------------------------------------------
void drv_set_baudrate(uint32_t baudRate, uint8_t *baudA, uint8_t *baudB, uint8_t *ctl )
{
}
//----------------------------------------------------------------------------------------
// UART RS232:
//----------------------------------------------------------------------------------------
void drv_uart_rs232_open( uint32_t baudrate )
{
	// El puerto de RS232 CONSOLA es el UART1

uint8_t bauddiv;

	switch(baudrate) {
	case 115200:
		bauddiv = 8;
		break;
	case 9600:
		bauddiv = 103;
		break;
	default:
		bauddiv = 103;
		break;
	}

	UBRR1L = bauddiv;
	UBRR1H = (bauddiv>>8);
	UCSR1B = _BV(RXCIE1) | _BV(RXEN1) | _BV(TXEN1);
	UCSR1C = ( serUCSRC_SELECT | serEIGHT_DATA_BITS );
	UCSR1A = _BV(U2X1);

	return;
}
//----------------------------------------------------------------------------------------
ISR( USART1_UDRE_vect )
{
	/* Handler (ISR) de TX1 de CONSOLA (BT/RS232)
	 * El handler maneja la trasmision de datos por la uart0.
	 * Para trasmitir, usando la funcion xUart0PutChar se encolan los datos y prende la
	 * flag de interrupcion.
	 * La rutina de interrupcion ejecuta este handler (SIGNAL) en el cual si hay
	 * datos en la cola los extrae y los trasmite.
	 * Si la cola esta vacia (luego del ultimo) apaga la interrupcion.
	*/

char cChar;
int8_t res = false;

	res = rBufferPop( &uart_rs232.TXringBuffer, (char *)&cChar );

	if( res == true ) {
		// Send the next character queued for Tx
		UDR1 = cChar;
	} else {
		// Queue empty, nothing to send.
		drv_uart_interruptOff(uart_rs232.uart_id);
	}
}
//----------------------------------------------------------------------------------------
ISR( USART1_RX_vect )
{
	/* Handler (ISR) de RX1 de CONSOLA ( BT/RS232)
	 * Este handler se encarga de la recepcion de datos.
	 * Cuando llega algun datos por el puerto serial lo recibe este handler y lo va
	 * guardando en la cola de recepcion
	 */

char cChar;

	cChar = UDR1;

	if( rBufferPokeFromISR( &uart_rs232.RXringBuffer, &cChar ) ) {
		taskYIELD();
	}
}
//----------------------------------------------------------------------------------------
// UART GPRS:
//----------------------------------------------------------------------------------------
void drv_uart_gprs_open( uint32_t baudrate )
{

uint8_t bauddiv;

	switch(baudrate) {
	case 115200:
		bauddiv = 8;
		break;
	case 9600:
		bauddiv = 103;
		break;
	default:
		bauddiv = 103;
		break;
	}

	UBRR0L = bauddiv;
	UBRR0H = (bauddiv>>8);
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = ( serUCSRC_SELECT | serEIGHT_DATA_BITS );
	UCSR0A = _BV(U2X0);

	return;
}
//----------------------------------------------------------------------------------------
ISR( USART0_UDRE_vect )
{
	/* Handler (ISR) de TX0 GPRS
	 * El handler maneja la trasmision de datos por la uart0.
	 * Para trasmitir, usando la funcion xUart0PutChar se encolan los datos y prende la
	 * flag de interrupcion.
	 * La rutina de interrupcion ejecuta este handler (SIGNAL) en el cual si hay
	 * datos en la cola los extrae y los trasmite.
	 * Si la cola esta vacia (luego del ultimo) apaga la interrupcion.
	*/

char cChar;
int8_t res = false;

	res = rBufferPop( &uart_gprs.TXringBuffer, (char *)&cChar );

	if( res == true ) {
		// Send the next character queued for Tx
		UDR0 = cChar;
	} else {
		// Queue empty, nothing to send.
		drv_uart_interruptOff(uart_gprs.uart_id);
	}
}
//----------------------------------------------------------------------------------------
ISR( USART0_RX_vect )
{
	/* Handler (ISR) de RX0 de GPRS
	 * Este handler se encarga de la recepcion de datos.
	 * Cuando llega algun datos por el puerto serial lo recibe este handler y lo va
	 * guardando en la cola de recepcion
	 */

char cChar;

	cChar = UDR0;

	if( rBufferPokeFromISR( &uart_gprs.RXringBuffer, &cChar ) ) {
		taskYIELD();
	}
}
//----------------------------------------------------------------------------------------


