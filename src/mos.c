/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	05/08/2022
 * 
 * Modinfo:
 * 11/07/2022:		Added mos_cmdDIR, mos_cmdLOAD, removed mos_cmdBYE
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE, mos_cmdDEL, improved command parsing and file error reporting
 * 14/07/2022:		Added mos_cmdRUN
 * 25/07/2022:		Added mos_getkey; variable keycode is now declared as a volatile
 * 03/08/2022:		Added a handful of MOS API calls
 * 05/08/2022:		Added mos_FEOF
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mos.h"
#include "uart.h"
#include "ff.h"

extern void exec16(long addr);

extern volatile char keycode;

t_mosFileObject	mosFileObjects[mos_maxOpenFiles];

static t_mosCommand mosCommands[] = {
	{ ".", 		&mos_cmdDIR },
	{ "DIR",	&mos_cmdDIR },
	{ "CAT",	&mos_cmdDIR },
	{ "LOAD",	&mos_cmdLOAD },
	{ "SAVE", 	&mos_cmdSAVE },
	{ "DEL", 	&mos_cmdDEL },
	{ "JMP",	&mos_cmdJMP },
	{ "RUN", 	&mos_cmdRUN },
	{ "CD", 	&mos_cmdCD },
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

char mos_getkey() {
	char ch = 0;
	while(ch == 0) {
		ch = keycode;
	}
//	while(keycode != 0);
	keycode = 0;
	return ch;
}

UINT24 mos_input(char * buffer, int bufferLength) {
	INT24 retval;
	putch(MOS_prompt);
	retval = mos_EDITLINE(buffer, bufferLength);
	printf("\n\r");
	return retval;
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

BOOL mos_parseNumber(char * ptr, UINT24 * p_Value) {
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
	
	fr = mos_DIR();
	mos_fileError(fr);
	return 0;
}

int mos_cmdLOAD(char * ptr) {
	FRESULT	fr;
	char *  filename;
	UINT24 	addr;
	
	if(
		!mos_parseString(NULL, &filename) ||
		!mos_parseNumber(NULL, &addr)
	) {
		return 1;
	}
	fr = mos_LOAD(filename, addr, 0);
	mos_fileError(fr);
	return 0;	
}

int mos_cmdSAVE(char * ptr) {
	FRESULT	fr;
	char *  filename;
	UINT24 	addr;
	UINT24 	size;
	
	if(
		!mos_parseString(NULL, &filename) ||
		!mos_parseNumber(NULL, &addr) ||
		!mos_parseNumber(NULL, &size)
	) {
		return 1;
	}
	fr = mos_SAVE(filename, addr, size);
	mos_fileError(fr);
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
	fr = mos_DEL(filename);
	mos_fileError(fr);
	return 0;
}

int mos_cmdJMP(char *ptr) {
	UINT24 	addr;
	void (* dest)(void) = 0;
	if(!mos_parseNumber(NULL, &addr)) {
		return 1;
	};
	dest = (void *)addr;
	dest();
	return 0;
}

int mos_cmdRUN(char *ptr) {
	UINT24 	addr;
	void (* dest)(void) = 0;
	if(!mos_parseNumber(NULL, &addr)) {
		return 1;
	};
	exec16(addr);
	return 0;
}

int mos_cmdCD(char * ptr) {
	char *  path;
	
	FRESULT	fr;
	
	if(
		!mos_parseString(NULL, &path) 
	) {
		return 1;
	}
	fr = f_chdir(path);
	mos_fileError(fr);
	return 0;
}

UINT24 mos_EDITLINE(char * buffer, int bufferLength) {
	char ch = 0;
	int index = 0;
	int limit = bufferLength - 1;
	
	while(ch != 13 && ch != 27) {
		ch = mos_getkey();
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
	return ch;
}

UINT24 mos_LOAD(char * filename, INT24 address, INT24 size) {
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	void * 	dest;
	FSIZE_t fSize;
	
	fr = f_open(&fil, filename, FA_READ);
	if(fr == FR_OK) {
		fSize = f_size(&fil);
		fr = f_read(&fil, (void *)address, fSize, &br);		
	}
	f_close(&fil);	
	return fr;
}

UINT24	mos_SAVE(char * filename, INT24 address, INT24 size) {
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	
	fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
	if(fr == FR_OK) {
		fr = f_write(&fil, (void *)address, size, &br);
	}
	f_close(&fil);	
	return fr;
}

UINT24	mos_CD(char *path) {
	FRESULT	fr;

	fr = f_chdir(path);
	return fr;
}

UINT24	mos_DIR(void) {
	FRESULT	fr;
	DIR	  	dir;
	static 	FILINFO  fno;
	int		yr, mo, da, hr, mi;
	char 	str[12];
	
	fr = f_getlabel("", str, 0);
	if(fr != 0) {
		return fr;
	}	
	printf("Volume: ");
	if(strlen(str) > 0) {
		printf("%s", str);
	}
	else {
		printf("<No Volume Label>");
	}
	printf("\n\r\n\r");
	
	fr = f_opendir(&dir, ".");
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
	f_closedir(&dir);
	return fr;
}

UINT24 mos_DEL(char * filename) {
	FRESULT	fr;	
	
	fr = f_unlink(filename);
	return fr;
}

UINT24 mos_FOPEN(char * filename, UINT8 mode) {
	FRESULT fr;
	int		i;
	
	for(i = 0; i < mos_maxOpenFiles; i++) {
		if(mosFileObjects[i].free == 0) {
			fr = f_open(&mosFileObjects[i].fileObject, filename, mode);
			if(fr == FR_OK) {
				mosFileObjects[i].free = 1;
				return i + 1;
			}
		}
	}
	return 0;
}

UINT24 mos_FCLOSE(UINT8 fh) {
	FRESULT fr;
	int 	i;
	
	if(fh > 0 && fh <= mos_maxOpenFiles) {
		i = fh - 1;
		if(&mosFileObjects[i].free > 0) {
			fr = f_close(&mosFileObjects[i].fileObject);
			mosFileObjects[i].free = 0;
		}
	}
	else {
		for(i = 0; i < mos_maxOpenFiles; i++) {
			if(mosFileObjects[i].free > 0) {
				fr = f_close(&mosFileObjects[i].fileObject);
				mosFileObjects[i].free = 0;
			}
		}
	}	
	return fh;	
}

char	mos_FGETC(UINT8 fh) {
	FRESULT fr;
	UINT	br;
	char	c;

	if(fh > 0 && fh <= mos_maxOpenFiles) {
		fr = f_read(&mosFileObjects[fh - 1].fileObject, &c, 1, &br); 
		if(fr == FR_OK) {
			return	c;
		}
	}
	return 0;
}

void	mos_FPUTC(UINT8 fh, char c) {
	if(fh > 0 && fh <= mos_maxOpenFiles) {
		f_putc(c, &mosFileObjects[fh - 1].fileObject);
	}
}

char	mos_FEOF(UINT8 fh) {
	if(fh > 0 && fh <= mos_maxOpenFiles) {
		if(f_eof(&mosFileObjects[fh - 1].fileObject) != 0) {
			return 1;
		}
	}
	return 0;
}