/*
 * sp5K.h
 *
 * Created on: 27/12/2013
 *      Author: root
 */

#ifndef SP5K_H_
#define SP5K_H_

#include <avr/io.h>			/* include I/O definitions (port names, pin names, etc) */
//#include <avr/signal.h>		/* include "signal" names (interrupt names) */
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <string.h>
#include <compat/deprecated.h>
#include <util/twi.h>
#include <util/delay.h>
#include <ctype.h>
#include <util/delay.h>
#include <avr/cpufunc.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "croutine.h"
#include "semphr.h"
#include "timers.h"
#include "limits.h"

#include "sp5KV6_global.h"

#include "frtos_io.h"

#include "l_cmdline.h"
#include "l_printf.h"
#include "l_eeprom.h"
#include "l_rtc1340.h"
#include "l_mcp.h"
#include "l_iopines.h"
#include "l_ads7828.h"

#define CHAR64		64
#define CHAR128	 	128
#define CHAR256	 	256

//----------------------------------------------------------------------------
// TASKS
/* Stack de las tareas */
#define tkCmd_STACK_SIZE		512
#define tkControl_STACK_SIZE	512
#define tkDigital_STACK_SIZE	384

//                            = 3370 + 400 = 3770 *2 = 7540: Memoria del heap usada para stack de tareas.
// El heap esta definido en 10K de modo que quedan 2460 libres para las uart queues y otros.
// La compilacion indica 4004 bytes o sea que quedan libres 12308 de los cuales 10000 se lleva el heap

/* Prioridades de las tareas */
#define tkCmd_TASK_PRIORITY	 		( tskIDLE_PRIORITY + 1 )
#define tkControl_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )
#define tkDigital_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )

//-----------------------------------------------------------------------------
typedef enum { DEBUG_NONE = 0, DEBUG_DIGITAL } debug_t;
typedef enum { USER_NORMAL, USER_TECNICO } usuario_t;

// Watchdog
typedef struct {
	uint8_t resetCause;
	uint8_t mcusr;
} wdgStatus_t;

// Entradas digitales
typedef struct {
	uint16_t pulse_count[NRO_DIGITAL_CHANNELS];			// 8
	float magnitud[NRO_DIGITAL_CHANNELS];				// 8
	uint8_t level[NRO_DIGITAL_CHANNELS];				// El canal tilt es el unico que vemos el nivel
} digital_inputs_t;		// 16 bytes

#define DLGID_LENGTH		7
#define PARAMNAME_LENGTH	5
#define IP_LENGTH			24
#define APN_LENGTH			32
#define PORT_LENGTH			7
#define SCRIPT_LENGTH		64
#define PASSWD_LENGTH		15
#define PARAMNAME_LENGTH	5

typedef struct {
	// Variables de trabajo.

//	char dlgId[DLGID_LENGTH];
//	char apn[APN_LENGTH];
//	char server_tcp_port[PORT_LENGTH];
//	char server_ip_address[IP_LENGTH];
//	char dlg_ip_address[IP_LENGTH];
//	char serverScript[SCRIPT_LENGTH];
//	char passwd[PASSWD_LENGTH];

	// Configuracion de Canales analogicos

	// Configuracion de canales digitales
	// Niveles logicos
	char d_ch_name[NRO_DIGITAL_CHANNELS][PARAMNAME_LENGTH];
	float d_ch_magpp[NRO_DIGITAL_CHANNELS];

	uint16_t timerPoll;
//	uint32_t timerDial;
	bool terminal_fixed_on;

//	uint8_t csq;
//	uint8_t dbm;
	debug_t debug;

	// El checksum DEBE ser el ultimo byte del systemVars !!!!
	uint8_t checksum;

} systemVars_t;

// VARIABLES GENERALES
systemVars_t systemVars;
TaskHandle_t xHandle_idle, xHandle_tkCmd, xHandle_tkControl, xHandle_tkDigital;
bool startTask;
wdgStatus_t wdgStatus;

//------------------------------------------------------------------------------------

xSemaphoreHandle sem_SYSVars;
StaticSemaphore_t SYSVARS_xMutexBuffer;
#define MSTOTAKESYSVARSSEMPH ((  TickType_t ) 10 )

// TASKS
void tkCmd(void * pvParameters);
void tkControl(void * pvParameters);
void tkDigital(void * pvParameters);

// TKDIGITAL
void pub_digital_read_counters( digital_inputs_t *dIn );
void pub_digital_load_defaults(void);
bool pub_digital_config_channel( uint8_t channel, char *chName, char *s_magPP );

// UTILS
void pub_save_configuration(void);
void pub_load_configuration(void);
void pub_load_defaults(void);

#endif /* SP5K_H_ */
