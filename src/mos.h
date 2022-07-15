/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	14/07/2022
 * 
 * Modinfo:
 * 11/07/2022:		Removed mos_cmdBYE, Added mos_cmdLOAD
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE
 * 14/07/2022:		Added mos_cmdRUN
 */

#ifndef MOS_H
#define MOS_H

typedef struct {
	char * name;
	int (*func)(char * ptr);
} t_mosCommand;

void 	mos_fileError(int error);

void	mos_input(char * buffer, int bufferLength);
void *	mos_getCommand(char * ptr);
void	mos_exec(char * buffer, int bufferLength);

BOOL 	mos_parseNumber(char * ptr, long * p_Value);
BOOL	mos_parseString(char * ptr, char ** p_Value);

int		mos_cmdDIR(char * ptr);
int		mos_cmdLOAD(char * ptr);
int 	mos_cmdSAVE(char *ptr);
int		mos_cmdDEL(char * ptr);
int		mos_cmdJMP(char * ptr);
int		mos_cmdRUN(char * ptr);

#endif MOS_H