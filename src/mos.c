/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	13/07/2022
 * 
 * Modinfo:
 * 11/07/2022:		Added mos_cmdDIR, mos_cmdLOAD, removed mos_cmdBYE
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE, mos_cmdDEL, improved command parsing and file error reporting
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mos.h"
#include "uart.h"
#include "ff.h"

#define MOS_prompt '*'

static t_mosCommand mosCommands[] = {
	{ ".", 		&mos_cmdDIR },
	{ "DIR",	&mos_cmdDIR },
	{ "CAT",	&mos_cmdDIR },
	{ "LOAD",	&mos_cmdLOAD },
	{ "SAVE", 	&mos_cmdSAVE },
	{ "DEL", 	&mos_cmdDEL },
	{ "JMP",	&mos_cmdJMP }
};

#define mosCommands_count (sizeof(mosCommands)/sizeof(t_mosCommand))

static char * mos_fileErrors[] = {
	"OK",
	"Error accessing SD card",
	"Assertion failed",
	"SD card failure",
	"Could not find file",
	"Could not find path",
	"Invalid path name",
	"Access denied due to prohibited access or directory full",
	"Access denied due to prohibited access",
	"Invalid file/directory object",
	"SD card is write protected",
	"Logical drive number is invalid",
	"Volume has no work area",
	"No valid FAT volume",
	"Error occurred during mkfs",
	"Volume timeout",
	"Volume locked",
	"LFN working buffer could not be allocated",
	"Too many open files",
	"Invalid parameter"
};

void mos_fileError(int error) {
	printf("%c%s\n\r", MOS_prompt, mos_fileErrors[error]);
}

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

BOOL mos_parseNumber(char * ptr, long * p_Value) {
	char * 	p = ptr;
	char * 	e;
	int 	base = 10;
	long 	value;

	p = strtok(p, " ");
	if(p == NULL) {
		return 0;
	}
	if(*p == '&') {
		base = 16;
		p++;
	}	
	value = strtol(p, &e, base);
	if(p == e) {
		return 0;
	}
	*p_Value = value;
	return 1;
}

BOOL mos_parseString(char * ptr, char ** p_Value) {
	char *	p = ptr;

	p = strtok(p, " ");
	if(p == NULL) {
		return 0;
	}
	*p_Value = p;
	return 1;
}

void mos_exec(char * buffer, int bufferLength) {
	char * 	ptr;
	int 	status;
	int 	(*func)(char * ptr);
	
	ptr = strtok(buffer, " ");
	if(ptr != NULL) {
		func = mos_getCommand(ptr);
		if(func != 0) {
			status = func(ptr);
			if(status != 0) {
				printf("Bad Parameters\n\r");
			}
		}
		else {
			printf("%cInvalid Command\n\r", MOS_prompt);
		}
	}
}

int mos_cmdDIR(char * ptr) {
	FRESULT	fr;
	DIR	  	dir;
	static 	FILINFO  fno;
	int		yr, mo, da, hr, mi;
	char 	str[12];
	
	fr = f_getlabel("", str, 0);
	if(fr != 0) {
		mos_fileError(fr);
		return 0;
	}
	
	printf("Volume: ");
	if(strlen(str) > 0) {
		printf("%s", str);
	}
	else {
		printf("<No Volume Label>");
	}
	printf("\n\r\n\r");
	
	fr = f_opendir(&dir, "/");
	if(fr == FR_OK) {
		for(;;) {
			fr = f_readdir(&dir, &fno);
			if (fr != FR_OK || fno.fname[0] == 0) {
				break;  // Break on error or end of dir
			}
			yr = (fno.fdate & 0xFE00) >>  9;	// Bits 15 to  9, from 1980
			mo = (fno.fdate & 0x01E0) >>  5;	// Bits  8 to  5
			da = (fno.fdate & 0x001F);			// Bits  4 to  0
			hr = (fno.ftime & 0xF800) >> 11;	// Bits 15 to 11
			mi = (fno.ftime & 0x07E0) >>  5;	// Bits 10 to  5
			
			printf("%04d/%02d/%02d\t%02d:%02d %c %*d %s\n\r", yr + 1980, mo, da, hr, mi, fno.fattrib & AM_DIR ? 'D' : ' ', 8, fno.fsize, fno.fname);
		}
	}
	else {
		mos_fileError(fr);	
	}
	f_closedir(&dir);
	printf("\n\r");
	return 0;
}

int mos_cmdLOAD(char * ptr) {
	char *  filename;
	long 	addr;
	
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	void * 	dest;
	FSIZE_t fSize;
	
	if(
		!mos_parseString(NULL, &filename) ||
		!mos_parseNumber(NULL, &addr)
	) {
		return 1;
	}
	
	dest = (void *)addr;
	
	fr = f_open(&fil, filename, FA_READ);
	if(fr == FR_OK) {
		fSize = f_size(&fil);
		fr = f_read(&fil, dest, fSize, &br);
		mos_fileError(fr);
	}
	else {
		mos_fileError(fr);
		return 0;
	}
	f_close(&fil);	
	return 0;
}

int mos_cmdSAVE(char * ptr) {
	char *  filename;
	long 	addr;
	long 	size;
	
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	void * 	dest;
	
	if(
		!mos_parseString(NULL, &filename) ||
		!mos_parseNumber(NULL, &addr) ||
		!mos_parseNumber(NULL, &size)
	) {
		return 1;
	}
	
	dest = (void *)addr;
	
	fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
	if(fr == FR_OK) {
		fr = f_write(&fil, dest, size, &br);
		mos_fileError(fr);
	}
	else {
		mos_fileError(fr);
		return 0;
	}
	f_close(&fil);	
	return 0;
}

int mos_cmdDEL(char * ptr) {
	char *  filename;
	
	FRESULT	fr;
	
	if(
		!mos_parseString(NULL, &filename) 
	) {
		return 1;
	}
	fr = f_unlink(filename);
	mos_fileError(fr);
	return 0;
}

int mos_cmdJMP(char *ptr) {
	long 	addr;
	void (* dest)(void) = 0;
	if(!mos_parseNumber(NULL, &addr)) {
		return 1;
	};
	dest = (void *)addr;
	dest();
	return 0;
}