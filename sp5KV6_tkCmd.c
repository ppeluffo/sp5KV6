/*
 * sp5K_tkCmd.c
 *
 *  Created on: 27/12/2013
 *      Author: root
 */

#include "sp5KV6.h"

//----------------------------------------------------------------------------------------
// FUNCIONES DE USO PRIVADO
//----------------------------------------------------------------------------------------
static void pv_snprintfP_OK(void );
static void pv_snprintfP_ERR(void);

static void pv_cmd_rwEE(uint8_t cmd_mode );
static void pv_cmd_rwRTC(uint8_t cmd_mode );
static void pv_cmd_rwMCP(uint8_t cmd_mode );
static void pv_cmd_rwGPRS(uint8_t cmd_mode );
static void pv_cmd_rdAN(uint8_t cmd_mode );

//----------------------------------------------------------------------------------------
// FUNCIONES DE CMDMODE
//----------------------------------------------------------------------------------------
static void cmdClearScreen(void);
static void cmdHelpFunction(void);
static void cmdResetFunction(void);
static void cmdStatusFunction(void);
static void cmdReadFunction(void);
static void cmdWriteFunction(void);
static void cmdKillFunction(void);
static void cmdConfigFunction(void);

#define WR_CMD 0
#define RD_CMD 1

static usuario_t tipo_usuario;
char buffer[32];
RtcTimeType_t rtc;
//----------------------------------------------------------------------------------------
void tkCmd(void * pvParameters)
{

uint8_t c;
uint8_t ticks;
( void ) pvParameters;

	while ( !startTask )
		vTaskDelay( ( TickType_t)( 100 / portTICK_RATE_MS ) );

	CMD_init();

	// Registro los comandos y los mapeo al cmd string.
	CMD_register((uint8_t *)"cls\0", cmdClearScreen );
	CMD_register((uint8_t *)"reset\0", cmdResetFunction);
	CMD_register((uint8_t *)"write\0", cmdWriteFunction);
	CMD_register((uint8_t *)"read\0", cmdReadFunction);
	CMD_register((uint8_t *)"help\0", cmdHelpFunction );
	CMD_register((uint8_t *)"status\0", cmdStatusFunction );
	CMD_register((uint8_t *)"config\0", cmdConfigFunction );
	CMD_register((uint8_t *)"kill\0", cmdKillFunction );

	// Fijo el timeout del READ
	ticks = 1;
	frtos_ioctl( fdRS232,ioctl_SET_TIMEOUT, &ticks );

	tipo_usuario = USER_TECNICO;

	RTC_start(&rtc);

	xprintf_P( PSTR("starting tkCmd..\r\n\0"));

	// loop
	for( ;; )
	{
		c = '\0';	// Lo borro para que luego del un CR no resetee siempre el timer.
		// el read se bloquea 50ms. lo que genera la espera.
		//while ( CMD_read( (char *)&c, 1 ) == 1 ) {
		while ( frtos_read( fdRS232, (char *)&c, 1 ) == 1 ) {
			CMD_process(c);
		}

	}
}
//------------------------------------------------------------------------------------
static void cmdClearScreen(void)
{
	// ESC [ 2 J
	xprintf_P( PSTR("\x1B[2J\0"));
}
//------------------------------------------------------------------------------------
static void cmdHelpFunction(void)
{
	CMD_makeArgv();

	// HELP WRITE
	if (!strcmp_P( strupr(argv[1]), PSTR("WRITE\0"))) {
		xprintf_P( PSTR("-write\r\n\0"));
		xprintf_P( PSTR("  rtc YYMMDDhhmm\r\n\0"));
		if ( tipo_usuario == USER_TECNICO ) {
			xprintf_P( PSTR("  ee {pos} {string}\r\n\0"));
			xprintf_P( PSTR("  leds,termpwr {on|off}\r\n\0"));
			xprintf_P( PSTR("  mcp {0|1} {regAddr} {regValue}\r\n\0"));
			xprintf_P( PSTR("  gprs {pwr|sw} {on|off}\r\n\0"));
			xprintf_P( PSTR("  clatch0,clatch1 {0|1}\r\n\0"));
			xprintf_P( PSTR("  {analog|sensor} pwr {on|off}\r\n\0"));
		}
		return;
	}

	// HELP READ
	else if (!strcmp_P( strupr(argv[1]), PSTR("READ\0"))) {
		xprintf_P( PSTR("-read\r\n\0"));
		xprintf_P( PSTR("  rtc, frame\r\n\0"));
		if ( tipo_usuario == USER_TECNICO ) {
			xprintf_P( PSTR("  ee {pos} {lenght}\r\n\0"));
			xprintf_P( PSTR("  termswitch, tilt, din0, din1\r\n\0"));
			xprintf_P( PSTR("  mcp {0|1} {regAddr}\r\n\0"));
			xprintf_P( PSTR("  ain {0..2}, bat, adc {0..7}\r\n\0"));

		}
		return;

	}

	// HELP RESET
	else if (!strcmp_P( strupr(argv[1]), PSTR("RESET\0"))) {
		xprintf_P( PSTR("-reset\r\n\0"));
		xprintf_P( PSTR("  memory {soft|hard}\r\n\0"));
		return;

	}

	// HELP CONFIG
	else if (!strcmp_P( strupr(argv[1]), PSTR("CONFIG\0"))) {
		xprintf_P( PSTR("-config\r\n\0"));
		xprintf_P( PSTR("  user {normal|tecnico}\r\n\0"));
		xprintf_P( PSTR("  default\r\n\0"));
		xprintf_P( PSTR("  save\r\n\0"));
		xprintf_P( PSTR("  debug {none|digital}\r\n\0"));
		xprintf_P( PSTR("  digital {0|1} dname magPP\r\n\0"));
		xprintf_P( PSTR("  terminal {on|off}\r\n\0"));
		return;

	}

	// HELP KILL
	else if (!strcmp_P( strupr(argv[1]), PSTR("KILL\0")) && ( tipo_usuario == USER_TECNICO) ) {
		xprintf_P( PSTR("-kill {digital}\r\n\0"));
		return;

	} else {

		// HELP GENERAL
		xprintf_P( PSTR("\r\nSpymovil %s %s %s %s\r\n\0"), SP5K_MODELO, SP5K_VERSION, SP5K_REV, SP5K_DATE);
		xprintf_P( PSTR("Available commands are:\r\n\0"));
		xprintf_P( PSTR("-cls\r\n\0"));
		xprintf_P( PSTR("-help\r\n\0"));
		xprintf_P( PSTR("-status\r\n\0"));
		xprintf_P( PSTR("-reset...\r\n\0"));
		xprintf_P( PSTR("-kill...\r\n\0"));
		xprintf_P( PSTR("-write...\r\n\0"));
		xprintf_P( PSTR("-read...\r\n\0"));
		xprintf_P( PSTR("-config...\r\n\0"));

	}

	xprintf_P( PSTR("\r\n\0"));

}
//------------------------------------------------------------------------------------
static void cmdResetFunction(void)
{

	CMD_makeArgv();

	// Reset memory ??
	if (!strcmp_P( strupr(argv[1]), PSTR("MEMORY\0"))) {

		if (!strcmp_P( strupr(argv[2]), PSTR("SOFT\0"))) {
//			FF_format(false );
		}
		if (!strcmp_P( strupr(argv[2]), PSTR("HARD\0"))) {
//			FF_format(true);
		}
	}

	wdt_enable(WDTO_30MS);
	while(1) {}
	cmdClearScreen();

}
//------------------------------------------------------------------------------------
static void cmdKillFunction(void)
{
	// Mata a una tarea para que no entorpezca con los logs de otras.
	// Se usa para tareas de debug. !!

	CMD_makeArgv();

	// KILL DIGITAL
	if (!strcmp_P( strupr(argv[1]), PSTR("DIGITAL\0"))) {
		vTaskSuspend( xHandle_tkDigital );
		pv_snprintfP_OK();
		return;
	}


	pv_snprintfP_ERR();
	return;

}
//------------------------------------------------------------------------------------
static void cmdStatusFunction(void)
{

uint8_t channel;

	xprintf_P( PSTR("\r\nSpymovil %s %s %s %s\r\n\0"), SP5K_MODELO, SP5K_VERSION, SP5K_REV, SP5K_DATE);

	// Fecha y Hora
	pv_cmd_rwRTC( RD_CMD );

	// CONFIG
	xprintf_P( PSTR(">Config:\r\n\0"));

	switch(systemVars.debug) {
	case DEBUG_NONE:
		xprintf_P( PSTR("  debug: none\r\n\0") );
		break;
	case DEBUG_DIGITAL:
		xprintf_P( PSTR("  debug: digital\r\n\0") );
		break;
	}

	/* Configuracion de canales digitales */
	for ( channel = 0; channel < NRO_DIGITAL_CHANNELS; channel++) {
		xprintf_P(PSTR("  d%d: {%.02f p/p} (%s)\r\n\0"), channel, systemVars.d_ch_magpp[channel],systemVars.d_ch_name[channel]);
	}

}
//------------------------------------------------------------------------------------
static void cmdReadFunction(void)
{

	CMD_makeArgv();

	// EE
	// read ee pos string
	if (!strcmp_P( strupr(argv[1]), PSTR("EE\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rwEE(RD_CMD);
		return;
	}

	// RTC
	// read rtc
	if (!strcmp_P( strupr(argv[1]), PSTR("RTC\0")) ) {
		pv_cmd_rwRTC(RD_CMD);
		return;
	}

	// TERMSWITCH
	// read termswitch
	if (!strcmp_P( strupr(argv[1]), PSTR("TERMSWITCH\0")) && ( tipo_usuario == USER_TECNICO) ) {
		xprintf_P( PSTR("TERMswitch=%d\r\n\0"), IO_read_TERM_PIN() );
		return;
	}

	// MCP
	// read mcp {0|1} {regAddr}
	if (!strcmp_P( strupr(argv[1]), PSTR("MCP\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rwMCP(RD_CMD);
		return;
	}

	// TILT
	// read tilt
	if (!strcmp_P( strupr(argv[1]), PSTR("TILT\0")) && ( tipo_usuario == USER_TECNICO) ) {
		xprintf_P( PSTR("TILT=%d\r\n\0"), IO_read_TILT_PIN() );
		return;
	}

	// DIN0
	// read din0
	if (!strcmp_P( strupr(argv[1]), PSTR("DIN0\0")) && ( tipo_usuario == USER_TECNICO) ) {
		xprintf_P( PSTR("DIN0=%d\r\n\0"), IO_read_DIN0() );
		return;
	}

	// DIN1
	// read din1
	if (!strcmp_P( strupr(argv[1]), PSTR("DIN1\0")) && ( tipo_usuario == USER_TECNICO) ) {
		xprintf_P( PSTR("DIN1=%d\r\n\0"), IO_read_DIN1() );
		return;
	}

	// AIN ( Entradas analogicas del datalogger )
	// read ain 0..2
	if (!strcmp_P( strupr(argv[1]), PSTR("AIN\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rdAN(0);
		return;
	}

	// BAT
	// read bat
	if (!strcmp_P( strupr(argv[1]), PSTR("BAT\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rdAN(1);
		return;
	}

	// ADC ( Caales del conversor A/D )
	// read adc {0..7}
	if (!strcmp_P( strupr(argv[1]), PSTR("ADC\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rdAN(2);
		return;
	}

	// CMD NOT FOUND
	xprintf_P( PSTR("ERROR\r\nCMD NOT DEFINED\r\n\0"));
	return;

}
//------------------------------------------------------------------------------------
static void cmdConfigFunction(void)
{

	CMD_makeArgv();

	// USER
	// config user {normal|tecnico}
	if (!strcmp_P( strupr(argv[1]), PSTR("USER\0"))) {
		if (!strcmp_P( strupr(argv[2]), PSTR("TECNICO\0"))) {
			tipo_usuario = USER_TECNICO;
			pv_snprintfP_OK();
			return;
		}
			if (!strcmp_P( strupr(argv[2]), PSTR("NORMAL\0"))) {
			tipo_usuario = USER_NORMAL;
			pv_snprintfP_OK();
			return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// DEBUG
	// config debug {none|digital}
	if (!strcmp_P( strupr(argv[1]), PSTR("DEBUG\0"))) {
		if (!strcmp_P( strupr(argv[2]), PSTR("NONE\0"))) {
			systemVars.debug = DEBUG_NONE;
			pv_snprintfP_OK();
		} else if (!strcmp_P( strupr(argv[2]), PSTR("DIGITAL\0"))) {
			systemVars.debug = DEBUG_DIGITAL;
			pv_snprintfP_OK();
		} else {
			pv_snprintfP_ERR();
		}
		return;
	}

	// DIGITAL
	// config digital {0|1} dname magPP
	if (!strcmp_P( strupr(argv[1]), PSTR("DIGITAL\0")) ) {
		if ( pub_digital_config_channel( atoi(argv[2]), argv[3], argv[4] ) ) {
			pv_snprintfP_OK();
		} else {
			pv_snprintfP_ERR();
		}
		return;
	}

	// SAVE
	// config save
	if (!strcmp_P( strupr(argv[1]), PSTR("SAVE\0"))) {
		pub_save_configuration();
		pv_snprintfP_OK();
		return;
	}

	// DEFAULT
	// config default
	if (!strcmp_P( strupr(argv[1]), PSTR("DEFAULT\0"))) {
		pub_load_defaults();
		pv_snprintfP_OK();
		return;
	}

	// TERMINAL
	// config terminal {on|off}
	if (!strcmp_P( strupr(argv[1]), PSTR("TERMINAL\0")) ) {
		if (!strcmp_P( strupr(argv[2]), PSTR("ON\0")) ) {
			systemVars.terminal_fixed_on = true;
			pv_snprintfP_OK();
			return;
		}
		if (!strcmp_P( strupr(argv[2]), PSTR("OFF\0")) ) {
			systemVars.terminal_fixed_on = false;
			pv_snprintfP_OK();
			return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// Opcion no reconocida
	pv_snprintfP_ERR();
	return;


}
//------------------------------------------------------------------------------------
static void cmdWriteFunction(void)
{

	CMD_makeArgv();

	// EE
	// write ee pos string
	if (!strcmp_P( strupr(argv[1]), PSTR("EE\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rwEE(WR_CMD);
		return;
	}

	// RTC
	// write rtc YYMMDDhhmm
	if (!strcmp_P( strupr(argv[1]), PSTR("RTC\0")) ) {
		pv_cmd_rwRTC(WR_CMD);
		return;
	}

	// LEDS
	// write leds (on|off)
	if (!strcmp_P( strupr(argv[1]), PSTR("LEDS\0")) ) {
		if (!strcmp_P( strupr(argv[2]), PSTR("ON\0")) ) {
			IO_led_KA_analogBoard_on();
			IO_led_KA_logicBoard_on();
			IO_led_MODEM_analogBoard_on();
			pv_snprintfP_OK();
			return;
		}
		if (!strcmp_P( strupr(argv[2]), PSTR("OFF\0")) ) {
			IO_led_KA_analogBoard_off();
			IO_led_KA_logicBoard_off();
			IO_led_MODEM_analogBoard_off();
			pv_snprintfP_OK();
			return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// TERMPWR
	// write termpwr (0|1)
	if (!strcmp_P( strupr(argv[1]), PSTR("TERMPWR\0")) ) {
		if (!strcmp_P( strupr(argv[2]), PSTR("ON\0")) ) {
			IO_set_term_pwr();
			return;
		}
		if (!strcmp_P( strupr(argv[2]), PSTR("OFF\0")) ) {
			IO_clear_term_pwr();
			return;
		}
		return;
	}

	// MCP
	// write mcp {0|1} {regAddr} {regValue}
	if (!strcmp_P( strupr(argv[1]), PSTR("MCP\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rwMCP(WR_CMD);
		return;
	}

	// GPRS
	// write gprs {pwr|sw} {on|off}
	if (!strcmp_P( strupr(argv[1]), PSTR("GPRS\0")) && ( tipo_usuario == USER_TECNICO) ) {
		pv_cmd_rwGPRS(WR_CMD);
		return;
	}

	// ANALOG
	// write analog pwr {on|off}
	if (!strcmp_P( strupr(argv[1]), PSTR("ANALOG\0")) && ( tipo_usuario == USER_TECNICO) ) {
		if (!strcmp_P( strupr(argv[2]), PSTR("PWR\0")) ) {
			if (!strcmp_P( strupr(argv[3]), PSTR("ON\0")) ) {
				IO_set_analog_pwr(); pv_snprintfP_OK(); return;
			}
			if (!strcmp_P( strupr(argv[3]), PSTR("OFF\0")) ) {
				IO_clear_analog_pwr(); pv_snprintfP_OK(); return;
			}
			pv_snprintfP_ERR();
			return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// SENSOR
	// write sensor pwr {on|off}
	if (!strcmp_P( strupr(argv[1]), PSTR("SENSOR\0")) && ( tipo_usuario == USER_TECNICO) ) {
		if (!strcmp_P( strupr(argv[2]), PSTR("PWR\0")) ) {
			if (!strcmp_P( strupr(argv[3]), PSTR("ON\0")) ) {
				IO_set_sensors_pwr(); pv_snprintfP_OK(); return;
			}
			if (!strcmp_P( strupr(argv[3]), PSTR("OFF\0")) ) {
				IO_clear_sensors_pwr(); pv_snprintfP_OK(); return;
			}
			pv_snprintfP_ERR();
			return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// CLATCH0
	// write clatch0 (0|1)
	if (!strcmp_P( strupr(argv[1]), PSTR("CLATCH0\0")) && ( tipo_usuario == USER_TECNICO) ) {
		if ( atoi(argv[2]) == 0 ) {
			IO_clear_CLRQ0(); pv_snprintfP_OK(); return;
		}
		if ( atoi(argv[2]) == 1 ) {
			IO_set_CLRQ0(); pv_snprintfP_OK(); return;
		}
		pv_snprintfP_ERR();
		return;
	}

	// CLATCH1
	// write clatch1 (0|1)
	if (!strcmp_P( strupr(argv[1]), PSTR("CLATCH1\0")) && ( tipo_usuario == USER_TECNICO) ) {
		if ( atoi(argv[2]) == 0 ) {
			IO_clear_CLRQ1(); pv_snprintfP_OK(); return;
		}
		if ( atoi(argv[2]) == 1 ) {
			IO_set_CLRQ1(); pv_snprintfP_OK(); return;
		}
		pv_snprintfP_ERR();
		return;
	}


	// CMD NOT FOUND
	xprintf_P( PSTR("ERROR\r\nCMD NOT DEFINED\r\n\0"));
	return;

}
//------------------------------------------------------------------------------------
// FUNCIONES PRIVADAS
//------------------------------------------------------------------------------------
static void pv_snprintfP_OK(void )
{
	xprintf_P( PSTR("ok\r\n\0"));
}
//------------------------------------------------------------------------------------
static void pv_snprintfP_ERR(void)
{
	xprintf_P( PSTR("error\r\n\0"));
}
//------------------------------------------------------------------------------------
static void pv_cmd_rwEE(uint8_t cmd_mode )
{

int xBytes = 0;
uint8_t length = 0;
char *p;

	// read ee {pos} {lenght}
	if ( cmd_mode == RD_CMD ) {
		xBytes = EE_read( (uint32_t)(atol(argv[2])), buffer, (uint8_t)(atoi(argv[3]) ) );
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:EE:pv_cmd_rwEE\r\n\0"));

		if ( xBytes > 0 ) {
			xprintf_P( PSTR( "%s\r\n\0"),buffer);
		}
		( xBytes > 0 ) ? pv_snprintfP_OK() : 	pv_snprintfP_ERR();
		return;
	}

	// write ee pos string
	if ( cmd_mode == WR_CMD ) {
		// Calculamos el largo del texto a escribir en la eeprom.
		p = argv[3];
		while (*p != 0) {
			p++;
			length++;
		}

		xBytes = EE_write( (uint32_t)(atol(argv[2])), argv[3], length );
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:EE:pv_cmd_rwEE\r\n\0"));

		( xBytes > 0 ) ? pv_snprintfP_OK() : 	pv_snprintfP_ERR();
		return;
	}

}
//------------------------------------------------------------------------------------
static void pv_cmd_rwRTC(uint8_t cmd_mode )
{

int8_t xBytes;

	if ( cmd_mode == WR_CMD ) {
		RTC_str2rtc(argv[2], &rtc);			// Convierto el string YYMMDDHHMM a RTC.
		xBytes = RTC_write_dtime(&rtc);		// Grabo el RTC
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:RTC:pv_cmd_rwRTC\r\n\0"));

		( xBytes > 0)? pv_snprintfP_OK() : 	pv_snprintfP_ERR();
		return;
	}

	if ( cmd_mode == RD_CMD ) {
		xBytes = RTC_read_dtime(&rtc);
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:RTC:pv_cmd_rwRTC\r\n\0"));

		RTC_rtc2str(buffer,&rtc);
		xprintf_P( PSTR("%s\r\n\0"), buffer );
		return;
	}

}
//------------------------------------------------------------------------------------
static void pv_cmd_rwMCP(uint8_t cmd_mode )
{
int xBytes = 0;
uint8_t data;

	// read mcp {0|1} {regAddr}
	if ( cmd_mode == RD_CMD ) {
		xBytes = MCP_read( (uint8_t)(atoi(argv[2])), (uint8_t)(atoi(argv[3]) ), (char *)&data, 1 );
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:MCP:pv_cmd_rwMCP\r\n\0"));

		if ( xBytes > 0 ) {
			xprintf_P( PSTR( "MCP%d(0x%02x)=0x%02x\r\n\0"),atoi(argv[2]), data);
		}
		( xBytes > 0 ) ? pv_snprintfP_OK() : 	pv_snprintfP_ERR();
		return;
	}

	// write mcp {0|1} {regAddr} {regValue}
	if ( cmd_mode == WR_CMD ) {

		xBytes = MCP_write( (uint8_t)(atol(argv[2])), (uint8_t)(atoi(argv[3]) ), argv[5], 1 );
		if ( xBytes == -1 )
			xprintf_P(PSTR("ERROR: I2C:MCP:pv_cmd_rwMCP\r\n\0"));

		( xBytes > 0 ) ? pv_snprintfP_OK() : 	pv_snprintfP_ERR();
		return;
	}

}
//------------------------------------------------------------------------------------
static void pv_cmd_rwGPRS(uint8_t cmd_mode )
{

	if ( cmd_mode == WR_CMD ) {

		// write gprs (pwr|sw) {on|off}

		if (!strcmp_P( strupr(argv[2]), PSTR("PWR\0")) ) {
			if (!strcmp_P( strupr(argv[3]), PSTR("ON\0")) ) {
				IO_gprs_pwr_on(); pv_snprintfP_OK(); return;
			}
			if (!strcmp_P( strupr(argv[3]), PSTR("OFF\0")) ) {
				IO_gprs_pwr_off(); pv_snprintfP_OK(); return;
			}
			pv_snprintfP_ERR();
			return;
		}

		if (!strcmp_P( strupr(argv[2]), PSTR("SW\0")) ) {
			if (!strcmp_P( strupr(argv[3]), PSTR("ON\0")) ) {
				IO_gprs_sw_on();
				pv_snprintfP_OK(); return;
			}
			if (!strcmp_P( strupr(argv[3]), PSTR("OFF\0")) ) {
				IO_gprs_sw_off(); pv_snprintfP_OK(); return;
			}
			pv_snprintfP_ERR();
			return;
		}

		return;
	}

}
//------------------------------------------------------------------------------------
static void pv_cmd_rdAN(uint8_t cmd_mode )
{

	// Lee el conversor A/D ya sea como canal, bateria o entrada analogica
	switch(cmd_mode) {
	case 0:	// read ain 0..2
		switch (atoi(argv[2]) ) {
		case 0:
			xprintf_P( PSTR("AIN0=%d\r\n\0"), AN_read_CH0() );
			break;
		case 1:
			xprintf_P( PSTR("AIN1=%d\r\n\0"), AN_read_CH1() );
			break;
		case 2:
			xprintf_P( PSTR("AIN2=%d\r\n\0"), AN_read_CH2() );
			break;
		default:
			pv_snprintfP_ERR();
			return;
		}
		break;

	case 1: // read bat
		xprintf_P( PSTR("BAT=%d\r\n\0"), AN_read_BAT() );
		break;

	case 2:	// read adc {0..7}
		xprintf_P( PSTR("ADC%d=%d\r\n\0"), atoi(argv[2]), ADC_read(atoi(argv[2])) );
		break;

	default:
		pv_snprintfP_ERR();
		return;
	}

}
//------------------------------------------------------------------------------------
