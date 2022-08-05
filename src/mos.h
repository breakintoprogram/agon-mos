/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	03/08/2022
 * 
 * Modinfo:
 * 11/07/2022:		Removed mos_cmdBYE, Added mos_cmdLOAD
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE
 * 14/07/2022:		Added mos_cmdRUN
 * 24/07/2022:		Added mos_getkey
 * 03/08/2022:		Added a handful of MOS API calls
 */

#ifndef MOS_H
#define MOS_H

#include "ff.h"

#define MOS_prompt '*'

#define mos_maxOpenFiles 8

typedef struct {
	char * name;
	int (*func)(char * ptr);
} t_mosCommand;

typedef struct {
	UINT8	free;
	FIL		fileObject;
} t_mosFileObject;

void 	mos_fileError(int error);

char	mos_getkey(void);
UINT24	mos_input(char * buffer, int bufferLength);
void *	mos_getCommand(char * ptr);
void	mos_exec(char * buffer, int bufferLength);

BOOL 	mos_parseNumber(char * ptr, UINT24 * p_Value);
BOOL	mos_parseString(char * ptr, char ** p_Value);

int		mos_cmdDIR(char * ptr);
int		mos_cmdLOAD(char * ptr);
int 	mos_cmdSAVE(char *ptr);
int		mos_cmdDEL(char * ptr);
int		mos_cmdJMP(char * ptr);
int		mos_cmdRUN(char * ptr);
int		mos_cmdCD(char * ptr);


UINT24	mos_EDITLINE(char * filename, int bufferLength);

UINT24	mos_LOAD(char * filename, INT24 address, INT24 size);
UINT24	mos_SAVE(char * filename, INT24 address, INT24 size);
UINT24	mos_CD(char * path);
UINT24	mos_DIR(void);
UINT24	mos_DEL(char * filename);

UINT24	mos_FOPEN(char * filename, UINT8 mode);
UINT24	mos_FCLOSE(UINT8 fh);
char	mos_FGETC(UINT8 fh);
void	mos_FPUTC(UINT8 fh, char c);

#endif MOS_H