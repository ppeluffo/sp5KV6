/*
 * l_cmdline.h
 *
 *  Created on: 11 ago. 2018
 *      Author: pablo
 *      Basado en la libreria de Pascal Stang.
 */

#ifndef SRC_SP5KLIBS_L_CMDLINE_H_
#define SRC_SP5KLIBS_L_CMDLINE_H_


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "l_printf.h"

// size of command database
// (maximum number of commands the cmdline system can handle)
#define CMDLINE_MAX_COMMANDS	10

// maximum length (number of characters) of each command string
// (quantity must include one additional byte for a null terminator)
#define CMDLINE_MAX_CMD_LENGTH	10

// allotted buffer size for command entry
// (must be enough chars for typed commands and the arguments that follow)
#define CMDLINE_BUFFERSIZE		64

// number of lines of command history to keep
// (each history buffer is CMDLINE_BUFFERSIZE in size)
// ***** ONLY ONE LINE OF COMMAND HISTORY IS CURRENTLY SUPPORTED
#define CMDLINE_HISTORYSIZE		1

// constants/macros/typdefs
typedef void (*CmdlineFuncPtrType)(void);

char *argv[16];

void CMD_init(void);
void CMD_register(uint8_t* newCmdString, CmdlineFuncPtrType newCmdFuncPtr);
void CMD_process(unsigned char c);
uint8_t CMD_makeArgv(void);

#endif /* SRC_SP5KLIBS_L_CMDLINE_H_ */
