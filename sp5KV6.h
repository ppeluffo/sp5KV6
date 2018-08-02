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

#include "frtos_io.h"
#include "FRTOS-CMD.h"

#include "l_printf.h"
#include "l_eeprom.h"
#include "l_rtc1340.h"


// DEFINICION DEL TIPO DE SISTEMA
//----------------------------------------------------------------------------
#define SP5K_REV "6.0.0beta"
#define SP5K_DATE "@ 20180801"
#define SP5KV5_8CH

#define SP5K_MODELO "sp5KV6 HW:avr1284P R5.0"
#define SP5K_VERSION "FW:FRTOS10"

#define CHAR64		64
#define CHAR128	 	128
#define CHAR256	 	256

//----------------------------------------------------------------------------
// TASKS
/* Stack de las tareas */
#define tkCmd_STACK_SIZE		512
#define tkControl_STACK_SIZE	512
//                            = 3370 + 400 = 3770 *2 = 7540: Memoria del heap usada para stack de tareas.
// El heap esta definido en 10K de modo que quedan 2460 libres para las uart queues y otros.
// La compilacion indica 4004 bytes o sea que quedan libres 12308 de los cuales 10000 se lleva el heap


/* Prioridades de las tareas */
#define tkCmd_TASK_PRIORITY	 		( tskIDLE_PRIORITY + 1 )
#define tkControl_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )

/* Prototipos de tareas */
void tkCmd(void * pvParameters);
void tkControl(void * pvParameters);

TaskHandle_t xHandle_idle, xHandle_tkCmd, xHandle_tkControl;

bool startTask;
typedef struct {
	uint8_t resetCause;
	uint8_t mcusr;
} wdgStatus_t;

wdgStatus_t wdgStatus;

//------------------------------------------------------------------------------------

typedef enum { USER_NORMAL, USER_TECNICO } usuario_t;


#endif /* SP5K_H_ */
