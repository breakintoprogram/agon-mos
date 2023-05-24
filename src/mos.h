/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	15/04/2023
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
 * 25/09/2022:		Added mos_GETERROR, mos_MKDIR
 * 13/10/2022:		Added mos_OSCLI and supporting code
 * 20/10/2022:		Tweaked error handling
 * 13/11/2022:		Added mos_cmp
 * 21/11/2022:		Added support for passing params to executables & ADL mode
 * 14/02/2023:		Added mos_cmdVDU
 * 20/02/2023:		Function mos_getkey now returns a BYTE
 * 09/03/2023:		Added mos_cmdTIME, mos_cmdCREDITS, mos_DIR now accepts a path
 * 14/03/2023:		Added mos_cmdCOPY and mos_COPY
 * 15/03/2023:		Added mos_GETRTC, mos_SETRTC
 * 21/03/2023:		Added mos_SETINTVECTOR
 * 14/04/2023:		Added fat_EOF
 * 15/04/2023:		Added mos_GETFIL, mos_FREAD, mos_FWRITE, mos_FLSEEK
 * 24/05/2023:		Added mos_cmdHELP, mos_cmdTYPE, mos_cmdCLS, mos_cmdMOUNT
 */

#ifndef MOS_H
#define MOS_H

#include "ff.h"

typedef struct {
	char * name;
	int (*func)(char * ptr);
	char * help;
} t_mosCommand;

typedef struct {
	UINT8	free;
	FIL		fileObject;
} t_mosFileObject;

void 	mos_error(int error);

BYTE	mos_getkey(void);
UINT24	mos_input(char * buffer, int bufferLength);
void *	mos_getCommand(char * ptr);
BOOL 	mos_cmp(char *p1, char *p2);
char *	mos_strtok(char *s1, char * s2);
char *	mos_strtok_r(char *s1, const char *s2, char **ptr);
int		mos_exec(char * buffer);
UINT8 	mos_execMode(UINT8 * ptr);

int	mos_mount(void);

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
int		mos_cmdCOPY(char *ptr);
int		mos_cmdMKDIR(char *ptr);
int		mos_cmdSET(char *ptr);
int		mos_cmdVDU(char *ptr);
int		mos_cmdTIME(char *ptr);
int		mos_cmdCREDITS(char *ptr);
int		mos_cmdTYPE(char *ptr);
int		mos_cmdCLS(char *ptr);
int		mos_cmdMOUNT(char *ptr);
int		mos_cmdHELP(char *ptr);

UINT24	mos_LOAD(char * filename, UINT24 address, UINT24 size);
UINT24	mos_SAVE(char * filename, UINT24 address, UINT24 size);
UINT24	mos_TYPE(char * filename);
UINT24	mos_CD(char * path);
UINT24	mos_DIR(char * path);
UINT24	mos_DEL(char * filename);
UINT24 	mos_REN(char * filename1, char * filename2);
UINT24	mos_COPY(char * filename1, char * filename2);
UINT24	mos_MKDIR(char * filename);
UINT24 	mos_BOOT(char * filename, char * buffer, UINT24 size);

UINT24	mos_FOPEN(char * filename, UINT8 mode);
UINT24	mos_FCLOSE(UINT8 fh);
UINT8	mos_FGETC(UINT8 fh);
void	mos_FPUTC(UINT8 fh, char c);
UINT24	mos_FREAD(UINT8 fh, UINT24 buffer, UINT24 btr);
UINT24	mos_FWRITE(UINT8 fh, UINT24 buffer, UINT24 btw);
UINT8  	mos_FLSEEK(UINT8 fh, UINT32 offset);
UINT8	mos_FEOF(UINT8 fh);

void 	mos_GETERROR(UINT8 errno, UINT24 address, UINT24 size);
UINT24 	mos_OSCLI(char * cmd);
UINT8 	mos_GETRTC(UINT24 address);
void	mos_SETRTC(UINT24 address);
UINT24	mos_SETINTVECTOR(UINT8 vector, UINT24 address);
UINT24	mos_GETFIL(UINT8 fh);

UINT8	fat_EOF(FIL * fp);

#define HELP_CAT	"*CAT <path> (Aliases: DIR andd .)\r\n"		\
			"Directory listing of the current directory.\r\n"

#define HELP_CD		"*CD <path>\r\n"				\
			"Change current directory\r\n"

#define HELP_COPY	"*COPY <filename1> <filename2>\r\n"		\
			"Create a copy of a file.\r\n"

#define HELP_CREDITS	"*CREDITS\r\n"					\
			"Output credits and version numbers for\r\n"	\
			"third-party libraries used in the Agon firmware\r\n"

#define HELP_DELETE	"*DELETE <filename> (Aliases: ERASE)\r\n"	\
			"Delete a file or folder (must be empty).\r\n"

#define HELP_JMP	"*JMP <addr>\r\n"				\
			"Jump to the specified address in memory\r\n"

#define HELP_LOAD	"*LOAD <filename> <addr>\r\n"			\
			"Load a file from the SD card to the specified"	\
			"address.\r\n"					\
			"If no parameters are passed, then `addr' will"	\
			"default to &40000.\r\n"

#define HELP_MKDIR	"*MKDIR <filename>\r\n"				\
			"Create a new folder on the SD card.\r\n"

#define HELP_RENAME	"*RENAME <filename1> <filename2> "		\
			"(Aliases: MOVE)\r\n"				\
			"Rename a file in the same folder.\r\n"

#define HELP_RUN	"*RUN <addr>\r\n"				\
			"Call an executable binary loaded in memory.\r\n"\
			"If no parameters are passed, then addr will "	\
			"default to &40000.\r\n"

#define HELP_SAVE	"*SAVE <filename> <addr> <size>\r\n"		\
			"Save a block of memory to the SD card\r\n"

#define HELP_SET	"*SET <option> <value>\r\n"			\
			"Set a system option\r\n\r\n"			\
			"Keyboard Layout\r\n"				\
			"SET KEYBOARD n: Set the keyboard layout\r\n"	\
			"    0: UK\r\n"					\
			"    1: US\r\n"					\
			"    2: German\r\n"				\
			"    3: Italian\r\n"				\
			"    4: Spanish\r\n"				\
			"    5: French\r\n"				\
			"    6: Belgian\r\n"				\
			"    7: Norwegian\r\n"				\
			"    8: Japanese\r\n"

#define HELP_TIME	"*TIME [ <yyyy> <mm> <dd> <hh> <mm> <ss> ]\r\n"	\
			"Set and read the ESP32 real-time clock\r\n"

#define HELP_VDU	"*VDU <char1> <char2> ... <charN>\r\n"		\
			"Write a stream of characters to the VDP\r\n"

#define HELP_TYPE	"*TYPE <filename>\r\n"				\
			"Display the contents of a file on the screen\r\n"

#define HELP_CLS	"*CLS\r\n"					\
			"Clear the screen\r\n"

#define HELP_MOUNT	"*MOUNT\r\n"					\
			"(Re-)mount the MicroSD card\r\n"

#define HELP_HELP	"*HELP [ <command> | all ]\r\n"			\
			"Display help on a single or all commands.\r\n"	\
			"List of commands:\r\n"				\
			"CAT, CD, COPY, CREDITS, DELETE, JMP, \r\n"	\
			"LOAD, MKDIR, RENAME, RUN, SAVE, SET, \r\n"	\
			"TIME, VDU, TYPE, CLS, MOUNT, HELP.\r\n"

#endif MOS_H
