/*
 * l_ringBuffer.c
 *
 *  Created on: 11 jul. 2018
 *      Author: pablo
 */


#include "l_ringBuffer.h"


//------------------------------------------------------------------------------------
bool rBufferPoke( ringBuffer_s *rB, char *cChar )
{

bool ret = false;

	taskENTER_CRITICAL();

	// Si el buffer esta vacio ajusto los punteros
	if( rB->count == 0) {
		rB->head = rB->tail = 0;
	}

	if ( rB->count < rB->length ) {
		rB->buff[rB->head] = *cChar;
		++rB->count;
		// Avanzo en modo circular
		rB->head = ( rB->head  + 1 ) % ( rB->length );
		ret = true;
    }

	taskEXIT_CRITICAL();
	return(ret);

}
//------------------------------------------------------------------------------------
bool rBufferPokeFromISR( ringBuffer_s *rB, char *cChar )
{

bool ret = false;

	// Si el buffer esta vacio ajusto los punteros
	if( rB->count == 0) {
		rB->head = rB->tail = 0;
	}

	if ( rB->count < rB->length ) {
		rB->buff[rB->head] = *cChar;
		++rB->count;
		// Avanzo en modo circular
		rB->head = ( rB->head  + 1 ) % ( rB->length );
		ret = true;
    }

	// Indico que estan llegando datos
	rB->arriving = true;

	return(ret);

}
//------------------------------------------------------------------------------------
bool rBufferPop( ringBuffer_s *rB, char *cChar )
{

bool ret = false;

	// Voy a leer un dato. Si estan llegando, espero.
	if ( rB->arriving == true ) {
		rB->arriving = false;
		vTaskDelay( ( TickType_t) ( 10 / portTICK_PERIOD_MS ) );
		// Si siguen llegando, me voy
		if ( rB->arriving == true ) {
			return(false);
		}
	}

	taskENTER_CRITICAL();

	//  Si el buffer esta vacio retorno.
	if( rB->count == 0) {
		rB->head = rB->tail = 0;
		taskEXIT_CRITICAL();
		return(ret);
	}

	*cChar = rB->buff[rB->tail];
	--rB->count;
	// Avanzo en modo circular
	rB->tail = ( rB->tail  + 1 ) % ( rB->length );
	ret = true;

	taskEXIT_CRITICAL();
	return(ret);
}
//------------------------------------------------------------------------------------
bool rBufferPopFromISR( ringBuffer_s *rB, char *cChar )
{

bool ret = false;

	//  Si el buffer esta vacio retorno.
	if( rB->count == 0) {
		rB->head = rB->tail = 0;
		return(ret);
	}

	*cChar = rB->buff[rB->tail];
	--rB->count;
	// Avanzo en modo circular
	rB->tail = ( rB->tail  + 1 ) % ( rB->length );
	ret = true;

	return(ret);
}
//------------------------------------------------------------------------------------
void rBufferFlush( ringBuffer_s *rB )
{

	rB->head = 0;
	rB->tail = 0;
	rB->count = 0;
	memset(rB->buff,'\0', rB->length );
}
//------------------------------------------------------------------------------------
void rBufferCreateStatic ( ringBuffer_s *rB, uint8_t *storage_area, uint16_t size  )
{
	rB->buff = storage_area;
	rB->head = 0;	// start
	rB->tail = 0;	// end
	rB->count = 0;
	rB->length = size;
	rB->arriving = false;
}
//------------------------------------------------------------------------------------
uint16_t rBufferGetCount( ringBuffer_s *rB )
{

	return(rB->count);

}
//------------------------------------------------------------------------------------
uint16_t rBufferGetFreeCount( ringBuffer_s *rB )
{

	return(rB->length - rB->count);

}
//------------------------------------------------------------------------------------
bool rBufferReachLowWaterMark( ringBuffer_s *rB)
{
bool retS = false;

	if ( rB->count  < (int)(0.2 * rB->length  ))
		retS = true;
	return(retS);

}
//------------------------------------------------------------------------------------
bool rBufferReachHighWaterMark( ringBuffer_s *rB )
{
bool retS = false;

	if ( rB->count  > (int)(0.8 * rB->length ))
		retS = true;

	return(retS);

}
//------------------------------------------------------------------------------------

