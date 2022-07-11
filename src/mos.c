/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	11/07/2022
 * 
 * Modinfo:
 * 11/07/2022:		Added mos_cmdDIR, mos_cmdLOAD, removed mos_cmdBYE
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
	{ "LOAD",	&mos_cmdLOAD }
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
	
	f_getlabel("", str, 0);
	
	printf("Volume: %s\n\r\n\r", str);
	
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
			printf("%04d/%02d/%02d\t%02d:%02d\t%s\n\r", yr + 1980, mo, da, hr, mi, fno.fname);
		}
	}
	f_closedir(&dir);
	return 0;
}

int mos_cmdLOAD(char * ptr) {
	char *  filename;
	char * 	addr;
	
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	void * 	dest;
	FSIZE_t fSize;
	
	filename = strtok(NULL, " ");
	addr = strtok(NULL, " ");
	if(filename == NULL || addr == NULL) {
		return 1;
	}

	dest = (void *)strtol(addr, NULL, 16);
	
	fr = f_open(&fil, filename, FA_READ);
	if(fr == FR_OK) {
		fSize = f_size(&fil);
		fr = f_read(&fil, dest, fSize, &br);
		if(fr == FR_OK) {
			printf("%cLoaded file %s to address 0x%06X, %d bytes\n\r", MOS_prompt, filename, dest, fSize);
		}
		else {
			printf("%cError loading file\n\r", MOS_prompt);
		}
	}
	else {
		printf("%cFile not found\n\r", MOS_prompt);
		return 0;
	}
	f_close(&fil);	

	return 0;
}