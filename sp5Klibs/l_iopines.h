/*
 * l_iopines.h
 *
 *  Created on: 18 de jun. de 2017
 *      Author: pablo
 */

// --------------------------------------------------------------------------------

#ifndef SRC_SP5KLIBS_L_IOPINES_H_
#define SRC_SP5KLIBS_L_IOPINES_H_

#include "l_mcp.h"
#include "sp5KV6_global.h"

//----------------------------------------------------------------------------
// Led KA placa logica
#define IO_set_led_KA_logicBoard() 		( MCP_set_bit ( 0, MCP0_GPIO, MCP0_GPIO_OLED ) )
#define IO_clear_led_KA_logicBoard() 	( MCP_clear_bit ( 0, MCP0_GPIO, MCP0_GPIO_OLED ) )

#define IO_led_KA_logicBoard_on()	IO_set_led_KA_logicBoard()
#define IO_led_KA_logicBoard_off()	IO_clear_led_KA_logicBoard()

// Led KA placa analogica SP5K_3CH ( PD6 )
#define LED_KA_AB_PORT		PORTD
#define LED_KA_AB_PIN		PIND
#define LED_KA_AB_BIT		6
#define LED_KA_AB_DDR		DDRD

#define IO_config_led_KA_analogBoard()	( LED_KA_AB_DDR |= (1 << LED_KA_AB_BIT) )
#define IO_set_led_KA_analogBoard()		( LED_KA_AB_PORT &= ~(1 << LED_KA_AB_BIT) )
#define IO_clear_led_KA_analogBoard()	( LED_KA_AB_PORT |= (1 << LED_KA_AB_BIT) )

#define IO_led_KA_analogBoard_on()	IO_set_led_KA_analogBoard()
#define IO_led_KA_analogBoard_off()	IO_clear_led_KA_analogBoard()

// Led MODEM placa analogica SP5K_3CH ( PC3 )
#define LED_MODEM_AB_PORT		PORTC
#define LED_MODEM_AB_PIN		PINC
#define LED_MODEM_AB_BIT		3
#define LED_MODEM_AB_DDR		DDRC

#define IO_config_led_MODEM_analogBoard()	( LED_MODEM_AB_DDR |= (1 << LED_MODEM_AB_BIT) )
#define IO_set_led_MODEM_analogBoard()		( LED_MODEM_AB_PORT &= ~(1 << LED_MODEM_AB_BIT) )
#define IO_clear_led_MODEM_analogBoard()	( LED_MODEM_AB_PORT |= (1 << LED_MODEM_AB_BIT) )

#define IO_led_MODEM_analogBoard_on()	IO_set_led_MODEM_analogBoard()
#define IO_led_MODEM_analogBoard_off()	IO_clear_led_MODEM_analogBoard()

// Control de power de la terminal
#define IO_set_term_pwr() 		( MCP_set_bit ( 0, MCP0_GPIO, MCP0_GPIO_OTERMPWR ) )
#define IO_clear_term_pwr() 	( MCP_clear_bit ( 0, MCP0_GPIO, MCP0_GPIO_OTERMPWR ) )

#define IO_term_pwr_on()	IO_set_term_pwr()
#define IO_term_pwr_off()	IO_clear_term_pwr()

// TERMINAL CONTROL SWITCH ( INPUT )
// Pin de control de fuente de la terminal ( PD7)
#define TERMSW_PORT		PORTD
#define TERMSW_PIN		PIND
#define TERMSW_BIT		7
#define TERMSW_DDR		DDRD
#define TERMSW_MASK		0x80

#define IO_config_TERM_PIN()	( TERMSW_DDR &= ~( 1 << TERMSW_BIT ))
#define IO_read_TERM_PIN()		( TERMSW_PIN & ( 1 << TERMSW_BIT ) ? 0 : 1 )

// UART CONTROL ( Buffer L365) ( OUTPUT )
#define UARTCTL_PORT	PORTD
#define UARTCTL_PIN		PIND
#define UARTCTL_BIT		4
#define UARTCTL_DDR		DDRD
//#define UARTCTL_MASK	0x10

#define IO_config_UART_CLT()	( UARTCTL_DDR |= ( 1 << UARTCTL_BIT) )
#define IO_set_UART_CTL()		( UARTCTL_PORT |= (1 << UARTCTL_BIT) )
#define IO_clear_UART_CTL()		( UARTCTL_PORT &= ~(1 << UARTCTL_BIT) )

#define IO_enable_uart_ctl()	IO_clear_UART_CTL()
#define IO_disable_uart_ctl()	IO_set_UART_CTL()

// Gprs control pin
#define IO_set_gprs_pwr() 		( MCP_set_bit ( 0, MCP0_GPIO, MCP0_GPIO_OGPRSPWR ) )
#define IO_clear_gprs_pwr() 	( MCP_clear_bit ( 0, MCP0_GPIO, MCP0_GPIO_OGPRSPWR ) )

#define IO_gprs_pwr_on()		IO_set_gprs_pwr()
#define IO_gprs_pwr_off()		IO_clear_gprs_pwr()

#define IO_set_gprs_sw() 		( MCP_set_bit ( 0, MCP0_GPIO, MCP0_GPIO_OGPRSPWR ) )
#define IO_clear_gprs_sw() 		( MCP_clear_bit ( 0, MCP0_GPIO, MCP0_GPIO_OGPRSPWR ) )

#define IO_gprs_sw_on()			IO_clear_gprs_sw()
#define IO_gprs_sw_off()		IO_set_gprs_sw()

#define IO_set_sensors_pwr()	( MCP_set_bit ( 1, MCP1_GPIOB, MCP1_GPIO_PWRSENSORS ) )
#define IO_clear_sensors_pwr()	( MCP_clear_bit ( 1, MCP1_GPIOB, MCP1_GPIO_PWRSENSORS ) )

#define IO_sensor_pwr_on()		IO_set_sensors_pwr()
#define IO_sensor_pwr_off()		IO_clear_sensors_pwr()

#define IO_set_analog_pwr()		( MCP_set_bit ( 1, MCP1_GPIOB, MCP1_GPIO_ANALOGPWR ) )
#define IO_clear_analog_pwr()	( MCP_clear_bit ( 1, MCP1_GPIOB, MCP1_GPIO_ANALOGPWR ) )

#define IO_analog_pwr_on()		IO_set_analog_pwr()
#define IO_analog_pwr_off()		IO_clear_analog_pwr()

// Digital inputs
#define IO_read_DIN0()		( MCP_read_bit (1, MCP1_GPIOB, MCP1_GPIO_DIN0))
#define IO_read_DIN1()		( MCP_read_bit (1, MCP1_GPIOB, MCP1_GPIO_DIN1))

// Latches ( Salidas digitales )
#define CLRQ0_PORT		PORTA
#define CLRQ0_PIN		PINA
#define CLRQ0_BIT		2
#define CLRQ0_DDR		DDRA
//#define CLRQ0_MASK		0x2

#define IO_config_CLRQ0()		( CLRQ0_DDR |= ( 1 << CLRQ0_BIT ) )
#define IO_set_CLRQ0()			( CLRQ0_PORT |= (1 << CLRQ0_BIT) )
#define IO_clear_CLRQ0()		( CLRQ0_PORT &= ~(1 << CLRQ0_BIT) )

#define CLRQ1_PORT		PORTA
#define CLRQ1_PIN		PINA
#define CLRQ1_BIT		3
#define CLRQ1_DDR		DDRA
//#define CLRQ1_MASK		0x3

#define IO_config_CLRQ1()		( CLRQ1_DDR |= ( 1 <<CLRQ1_BIT) )
#define IO_set_CLRQ1()			( CLRQ1_PORT |= (1 << CLRQ1_BIT) )
#define IO_clear_CLRQ1()		( CLRQ1_PORT &= ~(1 << CLRQ1_BIT) )

// TILT ( entrada digital )
#define TILT_PORT		PORTB
#define TILT_PIN		PINB
#define TILT_BIT		0
#define TILT_DDR		DDRB
//#define TILT_MASK		0x1

#define IO_config_TILT_PIN()	( TILT_DDR &= ~( 1 << TILT_BIT ))
#define IO_read_TILT_PIN()		( TILT_PIN & ( 1 << TILT_BIT ) ? 0 : 1 )

// OUTPUTS_1
#define IO_set_ENA1()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_ENA1 ) )
#define IO_clear_ENA1() 	( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_ENA1 ) )
#define IO_set_ENB1()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_ENB1 ) )
#define IO_clear_ENB1()		( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_ENB1 ) )
#define IO_set_PHA1()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_PHA1 ) )
#define IO_clear_PHA1()		( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_PHA1 ) )
#define IO_set_PHB1()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_PHB1 ) )
#define IO_clear_PHB1()		( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_PHB1 ) )

// OUTPUTS_2
#define IO_set_ENA2()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_ENA2 ) )
#define IO_clear_ENA2() 	( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_ENA2 ) )
#define IO_set_ENB2()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_ENB2 ) )
#define IO_clear_ENB2()		( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_ENB2 ) )
#define IO_set_PHA2()		( MCP_set_bit ( 1, MCP1_OLATB, MCP1_PHA2 ) )
#define IO_clear_PHA2()		( MCP_clear_bit ( 1, MCP1_OLATB, MCP1_PHA2 ) )
#define IO_set_PHB2()		( MCP_set_bit ( 1, MCP1_OLATA, MCP1_PHB2 ) )
#define IO_clear_PHB2()		( MCP_clear_bit ( 1, MCP1_OLATA, MCP1_PHB2 ) )

#define IO_set_RES()		( MCP_set_bit ( 1, MCP1_OLATB, MCP1_RESET ) )
#define IO_clear_RES()		( MCP_clear_bit ( 1, MCP1_OLATB, MCP1_RESET ) )
#define IO_set_SLP()		( MCP_set_bit ( 1, MCP1_OLATB, MCP1_SLEEP ) )
#define IO_clear_SLP()		( MCP_clear_bit ( 1, MCP1_OLATB, MCP1_SLEEP ) )

//----------------------------------------------------------------------------

#endif /* SRC_SP5KLIBS_L_IOPINES_H_ */
