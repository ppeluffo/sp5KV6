/*
 * sp5KV6_utils.c
 *
 *  Created on: 12 ago. 2018
 *      Author: pablo
 */

#include "sp5KV6.h"

//-----------------------------------------------------------------------------
void pub_save_configuration(void)
{
	// Guarda la configuracion en la memoria EE interna del micro.

}
//-----------------------------------------------------------------------------
void pub_load_configuration(void)
{
	// Lee y restaura la configuracion de la memoria EE interna del micro.

}
//-----------------------------------------------------------------------------
void pub_load_defaults(void)
{
	// Lee los parametros por defecto del sistema.

	systemVars.debug = DEBUG_NONE;
	systemVars.terminal_fixed_on = false;
	systemVars.timerPoll = 300;

	pub_digital_load_defaults();

}
//-----------------------------------------------------------------------------
