/*
 * sp5KV6_tkDigital.c
 *
 *  Created on: 11 ago. 2018
 *      Author: pablo
 *
 *  La nueva modalidad es por poleo.
 *  Configuro el MCP para que no interrumpa
 *  C/100ms leo el registro GPIO del MCP.
 *  En reposo la salida de los latch es 1 por lo que debo detectar cuando se hizo 0.
 *  Para evitar poder quedar colgado, c/ciclo borro el latch.
 *  Esto implica que no importa la duracion del pulso ya que lo capturo con un flip-flop, pero
 *  no pueden venir mas rapido que 10/s.
 *
 *	Esta version solo puede usarse con placas SP5K_3CH que tengan latch para los pulsos, o sea
 *	version >= R003.
 *
 */

#include "sp5KV6.h"

static void pv_clearLatches(void);
static void pv_poll_digitaInputs(void);
static void pv_digital_init(void);

static digital_inputs_t l_din;

//------------------------------------------------------------------------------------
void tkDigital(void * pvParameters)
{

( void ) pvParameters;
uint32_t waiting_ticks;
TickType_t xLastWakeTime;


	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	xprintf_P(PSTR("starting tkDigitalIn..\r\n\0"));

	// Inicializo los latches borrandolos
	pv_digital_init();

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    waiting_ticks = (uint32_t)(100) / portTICK_RATE_MS;

	for( ;; )
	{

		// Cada 100 ms leo las entradas digitales. fmax=10Hz
		//vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );
		vTaskDelayUntil( &xLastWakeTime, waiting_ticks );

		// Poleo las entradas
		pv_poll_digitaInputs();
	}

}
//------------------------------------------------------------------------------------
static void pv_digital_init(void)
{
uint8_t i;

	pv_clearLatches();

	for ( i = 0; i < NRO_DIGITAL_CHANNELS; i++ ) {
		l_din.level[i] = 0;
		l_din.pulse_count[i] = 0;
		l_din.magnitud[i] = 0.0;
	}

}
//------------------------------------------------------------------------------------
static void pv_poll_digitaInputs(void)
{

	// Leo los latches. Si estan en 0 es que latchearon un pulso por lo que incremento
	// los contadores.
	// Al salir los reseteo.

bool debug_flag = false;

	// Leo los pulsos.
	l_din.level[0] = IO_read_DIN0();
	l_din.level[1] = IO_read_DIN1();

	// Los flancos son de bajada.
	debug_flag = false;

	if ( l_din.level[0] == 0 ) {
		l_din.pulse_count[0]++;
		debug_flag = true;
	}

	if ( l_din.level[1] == 0 ) {
		l_din.pulse_count[1]++;
		debug_flag = true;
	}

	if ( (systemVars.debug == DEBUG_DIGITAL) && debug_flag ) {
		xprintf_P ( PSTR("DIGITAL: poll {p0=%d,p1=%d}\r\n\0"),l_din.pulse_count[0], l_din.pulse_count[1] );
	}

	// Siempre borro los latches para evitar la posibilidad de quedar colgado.
	pv_clearLatches();

	return;

}
//------------------------------------------------------------------------------------
static void pv_clearLatches(void)
{
	// Pongo un pulso 1->0->1 en Q0/Q1 pin para resetear el latch
	// En reposo debe quedar en H.
	IO_clear_CLRQ0();
	IO_clear_CLRQ1();

	taskYIELD();
	//_delay_us(5);
	//asm("nop");

	IO_set_CLRQ0();
	IO_set_CLRQ1();
}
//------------------------------------------------------------------------------------
// FUNCIONES PUBLICAS
//------------------------------------------------------------------------------------
void pub_digital_read_counters( digital_inputs_t *dIn )
{
	// Esta funcion es invocada SOLO desde tkData que es la que marca el ritmo
	// del poleo.
	// Calcula los caudales y copio los valores de los contadores en la estructura dIn.
	// De este modo quedan sincronizados los valores digitales al intervalo en que se midieron.

	// Calculo el Caudal por pulsos.
	if ( systemVars.timerPoll != 0 ) {
		l_din.magnitud[0] = l_din.pulse_count[0] * systemVars.d_ch_magpp[0] * 3600 / systemVars.timerPoll;
		l_din.magnitud[1] = l_din.pulse_count[1] * systemVars.d_ch_magpp[1] * 3600 / systemVars.timerPoll;
	}

	l_din.pulse_count[0] = 0;
	l_din.pulse_count[1] = 0;

	dIn->magnitud[0] = l_din.magnitud[0];
	dIn->magnitud[1] = l_din.magnitud[1];
}
//------------------------------------------------------------------------------------
bool pub_digital_config_channel( uint8_t channel, char *chName, char *s_magPP )
{

	// Configuro un canal digital. Lo uso para configurar desde modo cmd u online por gprs.

	if ( ( channel < 0) || ( channel >= NRO_DIGITAL_CHANNELS ) ) {
		return(false);
	}

	while ( xSemaphoreTake( sem_SYSVars, ( TickType_t ) 1 ) != pdTRUE )
		taskYIELD();

	if ( chName != NULL ) {
		memset( systemVars.d_ch_name[channel], '\0',   PARAMNAME_LENGTH );
		memcpy( systemVars.d_ch_name[channel], chName , ( PARAMNAME_LENGTH - 1 ));
	}

	if ( s_magPP != NULL ) {
		if ( atof(s_magPP) == 0 ) {
			systemVars.d_ch_magpp[channel] = 0.1;
		} else {
			systemVars.d_ch_magpp[channel] = atof(s_magPP);
		}
	}

	xSemaphoreGive( sem_SYSVars );

	return(true);

}
//----------------------------------------------------------------------------------------
void pub_digital_load_defaults(void)
{
	// Realiza la configuracion por defecto de los canales digitales.

	strncpy_P(systemVars.d_ch_name[0], PSTR("v0\0"),3);
	systemVars.d_ch_magpp[0] = 0.1;
	strncpy_P(systemVars.d_ch_name[1], PSTR("v1\0"),3);
	systemVars.d_ch_magpp[1] = 0.1;

}
//------------------------------------------------------------------------------------
