/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	10/07/2022
 * 
 * Modinfo:
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <string.h>

#include "mos.h"
#include "uart.h"

#define MOS_prompt '*'

static t_mosCommand mosCommands[] = {
	{ "BYE", &mos_cmdBYE },
	{ "DIR", &mos_cmdDIR }		
};

#define mosCommands_count (sizeof(mosCommands)/sizeof(t_mosCommand))

void mos_input(char * buffer, int bufferLength) {
	INT ch = 0;
	int index = 0;
	int limit = bufferLength - 1;
	
	putch(MOS_prompt);
	
	while(ch != 13) {
		ch = getch();

		if(ch > 0) {
			if(ch >= 32 && ch <= 126) {
				if(index < limit) {
					putch(ch);
					buffer[index] = ch;
					index++;
				}
			}
			else {				
				switch(ch) {
					case 127:	// Backspace
						if(index > 0) {
							putch(ch);
							buffer[index--] = 0;							
						}
						break;
				}					
			}
		}
	}
	buffer[index] = 0x00;
	printf("\n\r");
}

void * mos_getCommand(char * ptr) {
	int	   i;
	t_mosCommand * cmd;	
	for(i = 0; i < mosCommands_count; i++) {
		cmd = &mosCommands[i];
		if(strcmp(cmd->name, ptr) == 0) {
			return cmd->func;
		}
	}
	return 0;
}

void mos_exec(char * buffer, int bufferLength) {
	char * 	ptr;
	int 	status;
	int 	(*func)(void);
	
	ptr = strtok(buffer, " ");
	func = mos_getCommand(ptr);
	if(func != 0) {
		status = func();
	}
	else {
		printf("%cInvalid Command\n\r", MOS_prompt);
	}
}

int mos_cmdBYE(void) {
	printf("%cCommand: BYE\n\r", MOS_prompt);
	return 0;
}

int mos_cmdDIR(void) {
	printf("%cCommand: DIR\n\r", MOS_prompt);
	return 0;
}