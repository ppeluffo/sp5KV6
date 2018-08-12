/*
 * l_cmdline.c
 *
 *  Created on: 11 ago. 2018
 *      Author: pablo
 */

#include "l_cmdline.h"

// defines
#define ASCII_BEL				0x07
#define ASCII_BS				0x08
#define ASCII_CR				0x0D
#define ASCII_LF				0x0A
#define ASCII_ESC				0x1B
#define ASCII_DEL				0x7F

#define VT100_ARROWUP			'A'
#define VT100_ARROWDOWN			'B'
#define VT100_ARROWRIGHT		'C'
#define VT100_ARROWLEFT			'D'

#define CMDLINE_HISTORY_SAVE	0
#define CMDLINE_HISTORY_PREV	1
#define CMDLINE_HISTORY_NEXT	2


// internal commands
void pv_CMD_repaint(void);
void pv_CMD_doHistory(uint8_t action);
void pv_CMD_processInputString(void);
void pv_CMD_printPrompt(void);
void pv_CMD_printError(void);
void pv_CMD_execute(void);

// command list
// -commands are null-terminated strings
static char CmdlineCommandList[CMDLINE_MAX_COMMANDS][CMDLINE_MAX_CMD_LENGTH];
// command function pointer list
static CmdlineFuncPtrType CmdlineFunctionList[CMDLINE_MAX_COMMANDS];
// number of commands currently registered
uint8_t CmdlineNumCommands;

static char CmdlineBuffer[CMDLINE_BUFFERSIZE];

uint8_t CmdlineBufferLength;
uint8_t CmdlineBufferEditPos;
uint8_t CmdlineInputVT100State;
static char CmdlineHistory[CMDLINE_HISTORYSIZE][CMDLINE_BUFFERSIZE];
CmdlineFuncPtrType CmdlineExecFunction;

// Functions
//------------------------------------------------------------------------------------
void CMD_init(void)
{
	// reset vt100 processing state
	CmdlineInputVT100State = 0;
	// initialize input buffer
	CmdlineBufferLength = 0;
	CmdlineBufferEditPos = 0;
	// initialize executing function
	CmdlineExecFunction = 0;
	// initialize command list
	CmdlineNumCommands = 0;
}
//------------------------------------------------------------------------------------
void CMD_register(uint8_t* newCmdString, CmdlineFuncPtrType newCmdFuncPtr)
{
	// add command string to end of command list
	strcpy(CmdlineCommandList[CmdlineNumCommands], (char *)newCmdString);
	// add command function ptr to end of function list
	CmdlineFunctionList[CmdlineNumCommands] = newCmdFuncPtr;
	// increment number of registered commands
	CmdlineNumCommands++;
}
//------------------------------------------------------------------------------------
void CMD_process(unsigned char c)
{
	uint8_t i;
	// process the received character

	// VT100 handling
	// are we processing a VT100 command?
	if(CmdlineInputVT100State == 2)
	{
		// we have already received ESC and [
		// now process the vt100 code
		switch(c)
		{
		case VT100_ARROWUP:
			pv_CMD_doHistory(CMDLINE_HISTORY_PREV);
			break;
		case VT100_ARROWDOWN:
			pv_CMD_doHistory(CMDLINE_HISTORY_NEXT);
			break;
		case VT100_ARROWRIGHT:
			// if the edit position less than current string length
			if(CmdlineBufferEditPos < CmdlineBufferLength)
			{
				// increment the edit position
				CmdlineBufferEditPos++;
				// move cursor forward one space (no erase)
				xputChar(ASCII_ESC);
				xputChar('[');
				xputChar(VT100_ARROWRIGHT);
			}
			else
			{
				// else, ring the bell
				xputChar(ASCII_BEL);
			}
			break;
		case VT100_ARROWLEFT:
			// if the edit position is non-zero
			if(CmdlineBufferEditPos)
			{
				// decrement the edit position
				CmdlineBufferEditPos--;
				// move cursor back one space (no erase)
				xputChar(ASCII_BS);
			}
			else
			{
				// else, ring the bell
				xputChar(ASCII_BEL);
			}
			break;
		default:
			break;
		}
		// done, reset state
		CmdlineInputVT100State = 0;
		return;
	}
	else if(CmdlineInputVT100State == 1)
	{
		// we last received [ESC]
		if(c == '[')
		{
			CmdlineInputVT100State = 2;
			return;
		}
		else
			CmdlineInputVT100State = 0;
	}
	else
	{
		// anything else, reset state
		CmdlineInputVT100State = 0;
	}

	// Regular handling
	if( (c >= 0x20) && (c < 0x7F) )
	{
		// character is printable
		// is this a simple append
		if(CmdlineBufferEditPos == CmdlineBufferLength)
		{
			// echo character to the output
			xputChar(c);
			// add it to the command line buffer
			CmdlineBuffer[CmdlineBufferEditPos++] = c;
			// update buffer length
			CmdlineBufferLength++;
		}
		else
		{
			// edit/cursor position != end of buffer
			// we're inserting characters at a mid-line edit position
			// make room at the insert point
			CmdlineBufferLength++;
			for(i=CmdlineBufferLength; i>CmdlineBufferEditPos; i--) {
				CmdlineBuffer[i] = CmdlineBuffer[i-1];
			}
			// insert character
			CmdlineBuffer[CmdlineBufferEditPos++] = c;
			// repaint
			pv_CMD_repaint();
			// reposition cursor
			for(i=CmdlineBufferEditPos; i<CmdlineBufferLength; i++)
				xputChar(ASCII_BS);
		}
	}
	// handle special characters
	else if(c == ASCII_CR)
	{
		// user pressed [ENTER]
		// echo CR and LF to terminal
		xputChar(ASCII_CR);
		xputChar(ASCII_LF);
		// add null termination to command
		CmdlineBuffer[CmdlineBufferLength++] = 0;
		CmdlineBufferEditPos++;
		// command is complete, process it
		pv_CMD_processInputString();
		pv_CMD_execute();

		// reset buffer
		CmdlineBufferLength = 0;
		CmdlineBufferEditPos = 0;
	}
	else if(c == ASCII_BS)
	{
		if(CmdlineBufferEditPos)
		{
			// is this a simple delete (off the end of the line)
			if(CmdlineBufferEditPos == CmdlineBufferLength)
			{
				// destructive backspace
				// echo backspace-space-backspace
				xputChar(ASCII_BS);
				xputChar(' ');
				xputChar(ASCII_BS);
				// decrement our buffer length and edit position
				CmdlineBufferLength--;
				CmdlineBufferEditPos--;
			}
			else
			{
				// edit/cursor position != end of buffer
				// we're deleting characters at a mid-line edit position
				// shift characters down, effectively deleting
				CmdlineBufferLength--;
				CmdlineBufferEditPos--;
				for(i=CmdlineBufferEditPos; i<CmdlineBufferLength; i++) {
					CmdlineBuffer[i] = CmdlineBuffer[i+1];
				}
				// repaint
				pv_CMD_repaint();
				// add space to clear leftover characters
				xputChar(' ');
				// reposition cursor
				for(i=CmdlineBufferEditPos; i<(CmdlineBufferLength+1); i++)
					xputChar(ASCII_BS);
			}
		}
		else
		{
			// else, ring the bell
			xputChar(ASCII_BEL);
		}
	}
	else if(c == ASCII_DEL)
	{
		// not yet handled
	}
	else if(c == ASCII_ESC)
	{
		CmdlineInputVT100State = 1;
	}
}
//------------------------------------------------------------------------------------
uint8_t CMD_makeArgv(void)
{
// A partir de la linea de comando, genera un array de punteros a c/token
//
char *token = NULL;
char parseDelimiters[] = " ";
int i = 0;

	// inicialmente todos los punteros deben apuntar a NULL.
	memset(argv, '\0', sizeof(argv) );

	// Genero los tokens delimitados por ' '.
	token = strtok(CmdlineBuffer, parseDelimiters);
	argv[i++] = token;

	while ( (token = strtok(NULL, parseDelimiters)) != NULL ) {
		argv[i++] = token;
		if (i == 16) break;
	}

	return(( i - 1));
}
//------------------------------------------------------------------------------------
// FUNCIONES PRIVADAS
//------------------------------------------------------------------------------------
void pv_CMD_repaint(void)
{
	uint8_t* ptr;
	uint8_t i;

	// carriage return
	xputChar(ASCII_CR);
	// print fresh prompt
	pv_CMD_printPrompt();
	// print the new command line buffer
	i = CmdlineBufferLength;
	ptr = (uint8_t *)CmdlineBuffer;
	while(i--) xputChar(*ptr++);

}
//------------------------------------------------------------------------------------
void pv_CMD_doHistory(uint8_t action)
{
	switch(action)
	{
	case CMDLINE_HISTORY_SAVE:
		// copy CmdlineBuffer to history if not null string
		if( strlen(CmdlineBuffer) )
			strcpy(CmdlineHistory[0], CmdlineBuffer);
		break;
	case CMDLINE_HISTORY_PREV:
		// copy history to current buffer
		strcpy(CmdlineBuffer, CmdlineHistory[0]);
//		strcpy(SP5K_CmdlineBuffer, CmdlineHistory[0]);
		// set the buffer position to the end of the line
		CmdlineBufferLength = strlen(CmdlineBuffer);
		CmdlineBufferEditPos = CmdlineBufferLength;
		// "re-paint" line
		pv_CMD_repaint();
		break;
	case CMDLINE_HISTORY_NEXT:
		break;
	}
}
//------------------------------------------------------------------------------------
void pv_CMD_printPrompt(void)
{
	// Muestra el prompt
	xnprint( "cmd>\0", sizeof("cmd>\0") );
}
//------------------------------------------------------------------------------------
void pv_CMD_printError(void)
{
	// Muestra el mensaje de error cuando no encontro el comando
	xnprint( "command not found\r\n\0", sizeof("command not found\r\n\0") );
}
//------------------------------------------------------------------------------------
void pv_CMD_processInputString(void)
{
	uint8_t cmdIndex;
	uint8_t i=0;

	// save command in history
	pv_CMD_doHistory(CMDLINE_HISTORY_SAVE);

	// find the end of the command (excluding arguments)
	// find first whitespace character in CmdlineBuffer
	while( !((CmdlineBuffer[i] == ' ') || (CmdlineBuffer[i] == 0)) ) i++;

	if(!i)
	{
		// command was null or empty
		// output a new prompt
		pv_CMD_printPrompt();
		// we're done
		return;
	}

	// search command list for match with entered command
	for(cmdIndex=0; cmdIndex<CmdlineNumCommands; cmdIndex++)
	{
		if( !strncmp(CmdlineCommandList[cmdIndex], CmdlineBuffer, i) )
		{
			// user-entered command matched a command in the list (database)
			// run the corresponding function
			CmdlineExecFunction = CmdlineFunctionList[cmdIndex];
			// new prompt will be output after user function runs
			// and we're done
			return;
		}
	}

	// if we did not get a match
	// output an error message
	pv_CMD_printError();
	// output a new prompt
	pv_CMD_printPrompt();
}
//------------------------------------------------------------------------------------
void pv_CMD_execute(void)
{
	// do we have a command/function to be executed
	if(CmdlineExecFunction)
	{
		// run it
		CmdlineExecFunction();
		// reset
		CmdlineExecFunction = 0;
		// output new prompt
		pv_CMD_printPrompt();
	}
}
//------------------------------------------------------------------------------------

