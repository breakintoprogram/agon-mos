/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	11/07/2022
 * 
 * Modinfo:
 * 11/07/2022:		Removed mos_cmdBYE, Added mos_cmdLOAD
 */

#ifndef MOS_H
#define MOS_H

typedef struct {
	char * name;
	int (*func)(char * ptr);
} t_mosCommand;

void	mos_input(char * buffer, int bufferLength);
void *	mos_getCommand(char * ptr);
void	mos_exec(char * buffer, int bufferLength);

int		mos_cmdDIR(char * ptr);
int		mos_cmdLOAD(char * ptr);

#endif MOS_H