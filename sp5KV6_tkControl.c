/*

 * sp5KV3_tkControl.c
 *
 *  Created on: 7/4/2015
 *      Author: pablo
 *
 *  Tareas de control generales del SP5K
 *  - Recibe un mensaje del timer del led para indicar si debe prender o apagarlo.
 */

#include "sp5KV6.h"


static void pv_tkControl_init(void);
static void pv_check_wdg(void);

//------------------------------------------------------------------------------------
void tkControl(void * pvParameters)
{

( void ) pvParameters;

	// Aqui solo controlo la terminal por lo que no me importa tanto el watchdog.
	// Lo controlo en LedTiltWdg

	pv_tkControl_init();
	xprintf_P( PSTR("-----------------\r\n\0"));
	xprintf_P( PSTR("starting tkControl..\r\n\0"));

	// Al comienzo leo este handle para asi usarlo para leer el estado de los stacks.
	xHandle_idle = xTaskGetIdleTaskHandle();

	// Loop
    for( ;; )
    {

 	   	// Espero 1 segundo para revisar todo.
        vTaskDelay( ( TickType_t)( 1000 / portTICK_RATE_MS ) );

         // Reviso los sistemas perifericos.
        pv_check_wdg();

    }

}
//------------------------------------------------------------------------------------
static void pv_check_wdg(void)
{
	wdt_reset();
}
//------------------------------------------------------------------------------------
// FUNCIONES DE INIT
//------------------------------------------------------------------------------------
static void pv_tkControl_init(void)
{

	// Habilito el buffer serial
	UARTCTL_DDR |= (_BV(UARTCTL));
	UARTCTL_PORT &= ~(1 << UARTCTL);


	// Habilito al resto de las tareas a arrancar.
	startTask = true;

}
//------------------------------------------------------------------------------------

