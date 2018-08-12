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
#include "l_iopines.h"

static void pv_tkControl_init(void);
static void pv_check_wdg(void);
static void pv_check_leds(void);

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
        vTaskDelay( ( TickType_t)( 3000 / portTICK_RATE_MS ) );

         // Reviso los sistemas perifericos.
        pv_check_wdg();
        pv_check_leds();

    }

}
//------------------------------------------------------------------------------------
static void pv_check_wdg(void)
{
	wdt_reset();
}
//------------------------------------------------------------------------------------
static void pv_check_leds(void)
{
	IO_led_KA_analogBoard_on();
	vTaskDelay( 1 );
	IO_led_KA_analogBoard_off();

#ifndef DEBUG_I2C
	IO_led_KA_logicBoard_on();
	vTaskDelay( 1 );
	IO_led_KA_logicBoard_off();
#endif

}
//------------------------------------------------------------------------------------
// FUNCIONES DE INIT
//------------------------------------------------------------------------------------
static void pv_tkControl_init(void)
{

	// Habilito el buffer serial ( UARTCTL salida ). Esto permite usar el puerto serial
	IO_enable_uart_ctl();

	// Inicializo el MCP0. Permite prender la terminal.
	MCP_init(0);
	MCP_init(1);

	// Prendo la terminal
	IO_term_pwr_on();

	// Habilito al resto de las tareas a arrancar.
	startTask = true;

}
//------------------------------------------------------------------------------------

