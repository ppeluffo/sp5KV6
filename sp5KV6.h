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

// DEFINICION DEL TIPO DE SISTEMA
//----------------------------------------------------------------------------
#define SP5K_REV "6.0.5"
#define SP5K_DATE "@ 20180730"
#define SP5KV5_8CH

#define SP5K_MODELO "sp5KV3 HW:avr1284P R5.0"
#define SP5K_VERSION "FW:FRTOS8"


#define CHAR64		64
#define CHAR128	 	128
#define CHAR256	 	256

//----------------------------------------------------------------------------
// TASKS
/* Stack de las tareas */
#define tkCmd_STACK_SIZE		448
#define tkControl_STACK_SIZE	544
#define tkDigitalIn_STACK_SIZE	448
#define tkAIn_STACK_SIZE		384
#define tkGprsRx_STACK_SIZE		448
#define tkOutputs_STACK_SIZE	448
#define tkGprs_STACK_SIZE		650
//                            = 3370 + 400 = 3770 *2 = 7540: Memoria del heap usada para stack de tareas.
// El heap esta definido en 10K de modo que quedan 2460 libres para las uart queues y otros.
// La compilacion indica 4004 bytes o sea que quedan libres 12308 de los cuales 10000 se lleva el heap


/* Prioridades de las tareas */
#define tkCmd_TASK_PRIORITY	 		( tskIDLE_PRIORITY + 1 )
#define tkControl_TASK_PRIORITY	 	( tskIDLE_PRIORITY + 1 )
#define tkDigitalIn_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )
#define tkAIn_TASK_PRIORITY 		( tskIDLE_PRIORITY + 1 )
#define tkGprs_TASK_PRIORITY 		( tskIDLE_PRIORITY + 1 )
#define tkGprsRx_TASK_PRIORITY 		( tskIDLE_PRIORITY + 1 )
#define tkOutputs_TASK_PRIORITY 	( tskIDLE_PRIORITY + 1 )

/* Prototipos de tareas */
void tkCmd(void * pvParameters);
void tkControl(void * pvParameters);
void tkDigitalIn(void * pvParameters);
void tkAnalogIn(void * pvParameters);
void tkAnalogInit(void);
void tkGprsTx(void * pvParameters);
void tkOutputs(void * pvParameters);
void tkGprsRx(void * pvParameters);

TaskHandle_t xHandle_idle, xHandle_tkCmd, xHandle_tkControl, xHandle_tkDigitalIn, xHandle_tkAIn, xHandle_tkGprs, xHandle_tkGprsRx, xHandle_tkOutputs;

bool startTask;
typedef struct {
	uint8_t resetCause;
	uint8_t mcusr;
} wdgStatus_t;

wdgStatus_t wdgStatus;

// Mensajes entre tareas
#define TK_READ_FRAME			0x02	// to tkAnalogIN: (mode service) read a frame
#define TK_FRAME_READY			0x08	//
#define TK_REDIAL				0x10	//

//------------------------------------------------------------------------------------

xSemaphoreHandle sem_SYSVars;
#define MSTOTAKESYSVARSSEMPH ((  TickType_t ) 10 )

typedef enum { ON = 0, OFF } t_onoff;
typedef enum { modoPWRSAVE_OFF = 0, modoPWRSAVE_ON } t_pwrSave;
typedef enum { D_NONE = 0, D_MEM, D_GPRS, D_ANALOG, D_DIGITAL , D_OUTPUT } t_debug;
typedef enum { T_APAGADA = 0, T_PRENDIDA = 1 } t_terminalStatus;
typedef enum { OUT_OFF = 0, OUT_CONSIGNA, OUT_NORMAL } t_outputs;
typedef enum { CONSIGNA_DIURNA = 0, CONSIGNA_NOCTURNA } t_consigna_aplicada;
typedef enum { MODEM_PRENDER = 0, MODEM_APAGAR, TERM_PRENDER, TERM_APAGAR } t_uart_ctl;

typedef enum { USER_NORMAL, USER_TECNICO } usuario_t;

#define DLGID_LENGTH		12
#define APN_LENGTH			32
#define PORT_LENGTH			7
#define IP_LENGTH			24
#define SCRIPT_LENGTH		64
#define PASSWD_LENGTH		15
#define PARAMNAME_LENGTH	5

#define MODO_DISCRETO ( (systemVars.timerDial > 0 ) ? true : false )

typedef struct {
	uint8_t hour;
	uint8_t min;
} time_t;

#ifdef SP5KV5_3CH
typedef struct {
	uint16_t pulse_count[NRO_DIGITAL_CHANNELS];			// 8
	float caudal[NRO_DIGITAL_CHANNELS];					// 8
	uint8_t level[NRO_DIGITAL_CHANNELS];				// El canal tilt es el unico que vemos el nivel
} dinData_t;		// 16 bytes

typedef struct {
	// size = 7+5+5+4+3*4+1 = 33 bytes
	RtcTimeType_t rtc;						// 7
	double analogIn[NRO_ANALOG_CHANNELS];	// 12
	dinData_t dIn;							// 16
	double batt;							// 4
} frameData_t;	// 39 bytes

typedef struct {
	uint8_t modo;
	uint8_t out_A;
	uint8_t out_B;
	time_t consigna_diurna;
	time_t consigna_nocturna;
	uint8_t consigna_aplicada;
} outputs_t;

typedef struct {
	uint8_t modo;
	time_t hora_start;
	time_t hora_fin;
} pwrsave_t;

#endif /* SP5KV5_3CH */


typedef struct {
	// Variables de trabajo.
	// Tamanio: 302 bytes para 3 canales.

	uint8_t dummyBytes;
	uint8_t initByte;

	char dlgId[DLGID_LENGTH];
	char apn[APN_LENGTH];
	char server_tcp_port[PORT_LENGTH];
	char server_ip_address[IP_LENGTH];
	char dlg_ip_address[IP_LENGTH];
	char serverScript[SCRIPT_LENGTH];
	char passwd[PASSWD_LENGTH];

	uint8_t csq;
	uint8_t dbm;
	uint8_t ri;
	uint8_t termsw;
	bool terminal_on;

	uint16_t timerPoll;
	uint32_t timerDial;

	uint8_t debugLevel;		// Indica que funciones debugear.
	uint8_t gsmBand;


} systemVarsType;	// 315 bytes

systemVarsType systemVars,tmpSV;

#define EEADDR_SV 32		// Direccion inicio de la EE de escritura del systemVars.

//------------------------------------------------------------------------------------
// FUNCIONES DE USO GENERAL.
//------------------------------------------------------------------------------------
// utils
void pub_uarts_ctl(uint8_t cmd);

#ifdef SP5KV5_3CH
void pub_configPwrSave(uint8_t modoPwrSave, char *s_startTime, char *s_endTime);
#endif /* SP5KV5_3CH */

bool pub_loadSystemParams(void);

bool pub_saveSystemParams(void);
void pub_loadDefaults(void);
bool pub_configTimerDial(char *s_tDial);
void pub_reset(void);
void pub_convert_str_to_time_t ( char *time_str, time_t *time_struct );

// tkControl
bool pub_control_terminal_is_on(void);
void pub_debug_print_wdg_timers(void);
void pub_debug_print_stack_watermarks(void);



char debug_printfBuff[CHAR64];

void debug_test_printf(void);
void debug_print_self_stack_watermark(uint8_t trace);

// WATCHDOG
#define WDG_CMD			0
#define WDG_CTL			1
#define WDG_DIN			2
#define WDG_AIN			3
#define WDG_OUT			4
#define WDG_GPRSRX		5
#define WDG_GPRS		6

#define NRO_WDGS		7

#ifndef outb
	#define	outb(addr, data)	addr = (data)
#endif
#ifndef inb
	#define	inb(addr)			(addr)
#endif
#ifndef outw
	#define	outw(addr, data)	addr = (data)
#endif
#ifndef inw
	#define	inw(addr)			(addr)
#endif
#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
	#define cbi(reg,bit)	reg &= ~(BV(bit))
#endif
#ifndef sbi
	#define sbi(reg,bit)	reg |= (BV(bit))
#endif
#ifndef cli
	#define cli()			__asm__ __volatile__ ("cli" ::)
#endif
#ifndef sei
	#define sei()			__asm__ __volatile__ ("sei" ::)
#endif

//------------------------------------------------------------------------------------

#endif /* SP5K_H_ */
