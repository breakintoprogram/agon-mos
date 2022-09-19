/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	05/09/2022
 * 
 * Modinfo:
 * 11/07/2022:		Removed mos_cmdBYE, Added mos_cmdLOAD
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE
 * 14/07/2022:		Added mos_cmdRUN
 * 24/07/2022:		Added mos_getkey
 * 03/08/2022:		Added a handful of MOS API calls
 * 05/08/2022:		Added mos_FEOF
 * 05/09/2022:		Added mos_cmdREN, mos_cmdBOOT; moved mos_EDITLINE into mos_editline.c
 */

#ifndef MOS_H
#define MOS_H

#include "ff.h"

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
int		mos_cmdREN(char *ptr);

UINT24	mos_LOAD(char * filename, INT24 address, INT24 size);
UINT24	mos_SAVE(char * filename, INT24 address, INT24 size);
UINT24	mos_CD(char * path);
UINT24	mos_DIR(void);
UINT24	mos_DEL(char * filename);
UINT24 	mos_REN(char * filename1, char * filename2);
UINT24 	mos_BOOT(char * filename, char * buffer, INT24 size);

UINT24	mos_FOPEN(char * filename, UINT8 mode);
UINT24	mos_FCLOSE(UINT8 fh);
char	mos_FGETC(UINT8 fh);
void	mos_FPUTC(UINT8 fh, char c);

char	mos_FEOF(UINT8 fh);

#endif MOS_H