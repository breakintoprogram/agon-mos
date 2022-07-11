/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	10/07/2022
 * 
 * Modinfo:
 */

#ifndef MOS_H
#define MOS_H

typedef struct {
	char * name;
	int (*func)(void);
} t_mosCommand;

void	mos_input(char * buffer, int bufferLength);
void *	mos_getCommand(char * ptr);
void	mos_exec(char * buffer, int bufferLength);

int		mos_cmdBYE(void);
int		mos_cmdDIR(void);

#endif MOS_H