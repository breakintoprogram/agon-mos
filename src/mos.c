/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	20/02/2023
 * 
 * Modinfo:
 * 11/07/2022:		Added mos_cmdDIR, mos_cmdLOAD, removed mos_cmdBYE
 * 12/07/2022:		Added mos_cmdJMP
 * 13/07/2022:		Added mos_cmdSAVE, mos_cmdDEL, improved command parsing and file error reporting
 * 14/07/2022:		Added mos_cmdRUN
 * 25/07/2022:		Added mos_getkey; variable keycode is now declared as a volatile
 * 03/08/2022:		Added a handful of MOS API calls
 * 05/08/2022:		Added mos_FEOF
 * 05/09/2022:		Added mos_cmdREN, mos_cmdBOOT; moved mos_EDITLINE into mos_editline.c, default args for LOAD and RUN commands
 * 25/09/2022:		Added mos_GETERROR, mos_MKDIR; mos_input now sets first byte of buffer to 0
 * 03/10/2022:		Added mos_cmdSET
 * 13/10/2022:		Added mos_OSCLI and supporting code
 * 20/10/2022:		Tweaked error handling
 * 08/11/2022:		Fixed return value bug in mos_cmdRUN
 * 13/11/2022:		Case insensitive command processing with abbreviations; mos_exec now runs commands off SD card
 * 19/11/2022:		Added support for passing params to executables & ADL mode
 * 14/02/2023:		Added mos_cmdVDU, support for more keyboard layouts in mos_cmdSET
 * 20/02/2023:		Function mos_getkey now returns a BYTE
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mos.h"
#include "config.h"
#include "mos_editor.h"
#include "uart.h"
#include "ff.h"

extern int exec16(UINT24 addr, char * params);	// In misc.asm
extern int exec24(UINT24 addr, char * params);	// In misc.asm

extern volatile char keycode;					// In globals.asm

static char * mos_strtok_ptr;	// Pointer for current position in string tokeniser

t_mosFileObject	mosFileObjects[MOS_maxOpenFiles];

// Array of MOS commands and pointer to the C function to run
//
static t_mosCommand mosCommands[] = {
	{ ".", 		&mos_cmdDIR },
	{ "DIR",	&mos_cmdDIR },
	{ "CAT",	&mos_cmdDIR },
	{ "LOAD",	&mos_cmdLOAD },
	{ "SAVE", 	&mos_cmdSAVE },
	{ "DELETE",	&mos_cmdDEL },
	{ "ERASE",	&mos_cmdDEL },
	{ "JMP",	&mos_cmdJMP },
	{ "RUN", 	&mos_cmdRUN },
	{ "CD", 	&mos_cmdCD },
	{ "RENAME",	&mos_cmdREN },
	{ "MKDIR", 	&mos_cmdMKDIR },
	{ "SET",	&mos_cmdSET },
	{ "VDU",	&mos_cmdVDU },
};

#define mosCommands_count (sizeof(mosCommands)/sizeof(t_mosCommand))

// Array of file errors; mapped by index to the error numbers returned by FatFS
//
static char * mos_errors[] = {
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
	"Invalid parameter",
	"Invalid command",
	"Invalid executable",
};

// Output a file error
// Parameters:
// - error: The FatFS error number
//
void mos_error(int error) {
	printf("\n\r%s\n\r", mos_errors[error]);
}

// Wait for a keycode character from the VPD
// Returns:
// - ASCII keycode
//
BYTE mos_getkey() {
	char ch = 0;
	while(ch == 0) {		// Loop whilst no key pressed
		ch = keycode;		// Variable keycode is updated by interrupt
	}
	keycode = 0;			// Reset keycode to debounce the key
	return ch;
}

// Call the line editor from MOS
// Parameters:
// - buffer: Pointer to the line edit buffer
// - bufferLength: Size of the line edit buffer in bytes
// Returns:
// - The keycode (ESC or CR)
//
UINT24 mos_input(char * buffer, int bufferLength) {
	INT24 retval;
	putch(MOS_prompt);
	retval = mos_EDITLINE(buffer, bufferLength, 1);
	printf("\n\r");
	return retval;
}

// Parse a MOS command from the line edit buffer
// Parameters:
// - ptr: Pointer to the MOS command in the line edit buffer
// Returns:
// - Function pointer, or 0 if command not found
//
void * mos_getCommand(char * ptr) {
	int	   i;
	t_mosCommand * cmd;	
	for(i = 0; i < mosCommands_count; i++) {
		cmd = &mosCommands[i];
		if(mos_cmp(cmd->name, ptr) == 0) {
			return cmd->func;
		}
	}
	return 0;
}

// Case insensitive commpare with abbreviations
// Parameters:
// - p1: The command to be compared against
// - p2: The inputted command
//
BOOL mos_cmp(const char *p1, const char *p2) {
	char c1;
	char c2;	
	do {		
		c1 = toupper(*p1++);
		c2 = toupper(*p2++);
		if(c2 == '.') {
			c1 = 0;
			c2 = 0;
		}
		if(c1 < 0x20) c1 = 0;
		if(c2 < 0x20) c2 = 0;
	} while(c1 && c2 && c1 == c2);
	return (const unsigned char*)c1 - (const unsigned char*)c2;
}

// String tokeniser
// Parameters:
// - s1: String to tokenise
// - s2: Delimiter
// - ptr: Pointer to store the current position in (mos_strtok_r)
// Returns:
// - Pointer to tokenised string
//
char * mos_strtok(char *s1, char * s2) {
	return mos_strtok_r(s1, s2, &mos_strtok_ptr);
}

char * mos_strtok_r(char *s1, const char *s2, char **ptr) {
	char *end;

	if (s1 == NULL) {
		s1 = *ptr;
	}
	
	if (*s1 == '\0') {
		*ptr = s1;
		return NULL;
    }
	// Scan leading delimiters
	//
	s1 += strspn(s1, s2);
	if (*s1 == '\0') {
		*ptr = s1;
		return NULL;
    }
	// Find the end of the token
	//
	end = s1 + strcspn(s1, s2);
	if (*end == '\0') {
      *ptr = end;
      return s1;
    }
	// Terminate the token and make *SAVE_PTR point past it
	//
	*end = '\0';
	*ptr = end + 1;
	
	return s1;
}

// Parse a number from the line edit buffer
// Parameters:
// - ptr: Pointer to the number in the line edit buffer
// - p_Value: Pointer to the return value
// Returns:
// - true if the function succeeded, otherwise false
//
BOOL mos_parseNumber(char * ptr, UINT24 * p_Value) {
	char * 	p = ptr;
	char * 	e;
	int 	base = 10;
	long 	value;

	p = mos_strtok(p, " ");
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

// Parse a string from the line edit buffer
// Parameters:
// - ptr: Pointer to the string in the line edit buffer
// - p_Value: Pointer to the return value
// Returns:
// - true if the function succeeded, otherwise false
//
BOOL mos_parseString(char * ptr, char ** p_Value) {
	char *	p = ptr;

	p = mos_strtok(p, " ");
	if(p == NULL) {
		return 0;
	}
	*p_Value = p;
	return 1;
}

// Execute a MOS command
// Parameters:
// - buffer: Pointer to a zero terminated string that contains the MOS command with arguments
// Returns:
// - MOS error code
//
int mos_exec(char * buffer) {
	char * 	ptr;
	int 	fr = 0;
	int 	(*func)(char * ptr);
	char	path[256];
	UINT8	mode;

	ptr = mos_strtok(buffer, " ");
	if(ptr != NULL) {
		func = mos_getCommand(ptr);
		if(func != 0) {
			fr = func(ptr);
		}
		else {
			sprintf(path, "/mos/%s.bin", ptr);
			fr = mos_LOAD(path, MOS_starLoadAddress, 0);
			if(fr == 0) {
				mode = mos_execMode((UINT8 *)MOS_starLoadAddress);
				switch(mode) {
					case 0:		// Z80 mode
						fr = exec16(MOS_starLoadAddress, mos_strtok_ptr);
						break;
					case 1: 	// ADL mode
						fr = exec24(MOS_starLoadAddress, mos_strtok_ptr);
						break;	
					default:	// Unrecognised header
						fr = 21;
						break;
				}
			}
			else {
				if(fr == 4) {
					fr = 20;
				}
			}
		}
	}
	return fr;
}

// Get the MOS Z80 execution mode
// Parameters:
// - ptr: Pointer to the code block
// Returns:
// - 0: Z80 mode
// - 1: ADL mode
//
UINT8 mos_execMode(UINT8 * ptr) {
	if(
		*(ptr+0x40) == 'M' &&
		*(ptr+0x41) == 'O' &&
		*(ptr+0x42) == 'S'
	) {
		return *(ptr+0x44);
	}
	return 0xFF;
}

// DIR command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdDIR(char * ptr) {	
	FRESULT	fr;

	fr = mos_DIR();
	return fr;
}

// LOAD <filename> <addr> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdLOAD(char * ptr) {
	FRESULT	fr;
	char *  filename;
	UINT24 	addr;
	
	if(
		!mos_parseString(NULL, &filename)
	) {
		return 19; // Bad Parameter
	}
	if(!mos_parseNumber(NULL, &addr)) addr = MOS_defaultLoadAddress;
	fr = mos_LOAD(filename, addr, 0);
	return fr;	
}

// SAVE <filename> <addr> <len> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
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
		return 19; // Bad Parameter
	}
	fr = mos_SAVE(filename, addr, size);
	return fr;
}

// DEL <filename> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdDEL(char * ptr) {
	char *  filename;
	
	FRESULT	fr;
	
	if(
		!mos_parseString(NULL, &filename) 
	) {
		return 19; // Bad Parameter
	}
	fr = mos_DEL(filename);
	return fr;
}

// JMP <addr> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdJMP(char *ptr) {
	UINT24 	addr;
	void (* dest)(void) = 0;
	if(!mos_parseNumber(NULL, &addr)) {
		return 19; // Bad Parameter
	};
	dest = (void *)addr;
	dest();
	return 0;
}

// RUN <addr> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdRUN(char *ptr) {
	int 	fr;
	UINT24 	addr;
	UINT8	mode;
	void (* dest)(void) = 0;
	
	if(!mos_parseNumber(NULL, &addr)) {
		addr = MOS_defaultLoadAddress;
	}
	mode = mos_execMode((UINT8 *)addr);
	switch(mode) {
		case 0:		// Z80 mode
			fr = exec16(addr, mos_strtok_ptr);
			break;
		case 1: 	// ADL mode
			fr = exec24(addr, mos_strtok_ptr);
			break;	
		default:	// Unrecognised header
			fr = 21;
			break;
	}	
	return fr;
}

// CD <path> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdCD(char * ptr) {
	char *  path;
	
	FRESULT	fr;
	
	if(
		!mos_parseString(NULL, &path) 
	) {
		return 19; // Bad Parameter
	}
	fr = f_chdir(path);
	return fr;
}

// REN <filename1> <filename2> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdREN(char *ptr) {
	FRESULT	fr;
	char *  filename1;
	char *	filename2;
	
	if(
		!mos_parseString(NULL, &filename1) ||
		!mos_parseString(NULL, &filename2)
	) {
		return 19; // Bad Parameter
	}
	fr = mos_REN(filename1, filename2);
	return fr;
}

// MKDIR <filename> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdMKDIR(char * ptr) {
	char *  filename;
	
	FRESULT	fr;
	
	if(
		!mos_parseString(NULL, &filename) 
	) {
		return 19; // Bad Parameter
	}
	fr = mos_MKDIR(filename);
	return fr;
}

// SET <option> <value> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdSET(char * ptr) {
	char *	command;
	UINT24 	value;
	
	if(
		!mos_parseString(NULL, &command) ||
		!mos_parseNumber(NULL, &value)
	) {
		return 19; // Bad Parameter
	}
	if(strcmp(command, "KEYBOARD") == 0 && value <= 8) {
		putch(0x17);
		putch(0x00);
		putch(0x01);
		putch(value & 0xFF);
		return 0;
	}
	return 19; // Bad Parameter
}

// VDU <char1> <char2> ... <charN>
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int	mos_cmdVDU(char *ptr) {
	UINT24 	value;
	
	while(mos_parseNumber(NULL, &value)) {
		if(value > 255) {
			return 19;	// Bad Parameter
		}
		putch(value);
	}
	return 0;
}

// Load a file from SD card to memory
// Parameters:
// - filename: Path of file to load
// - address: Address in RAM to load the file into
// - size: Number of bytes to load
// Returns:
// - FatFS return code
// 
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

// Save a file from memory to SD card
// Parameters:
// - filename: Path of file to save
// - address: Address in RAM to save the file from
// - size: Number of bytes to save
// Returns:
// - FatFS return code
// 
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

// Change directory
// Parameters:
// - filename: Path of file to save
// Returns:
// - FatFS return code
// 
UINT24	mos_CD(char *path) {
	FRESULT	fr;

	fr = f_chdir(path);
	return fr;
}

// Directory listing
// Returns:
// - FatFS return code
// 
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

// Delete file
// Parameters:
// - filename: Path of file to delete
// Returns:
// - FatFS return code
// 
UINT24 mos_DEL(char * filename) {
	FRESULT	fr;	
	
	fr = f_unlink(filename);
	return fr;
}

// Rename file
// Parameters:
// - filename1: Path of file to rename
// - filename2: New filename
// Returns:
// - FatFS return code
// 
UINT24 mos_REN(char * filename1, char * filename2) {
	FRESULT fr;
	
	fr = f_rename(filename1, filename2);
	return fr;
}

// Make a directory
// Parameters:
// - filename: Path of file to delete
// Returns:
// - FatFS return code
// 
UINT24 mos_MKDIR(char * filename) {
	FRESULT	fr;	
	
	fr = f_mkdir(filename);
	return fr;
}

// Load and run the config file 
// Parameters:
// - filename: The config file to execute
// - buffer: Storage for each line to be loaded into and executed from (recommend 256 bytes)
// - size: Size of buffer (in bytes)
// Returns:
// - FatFS return code
//
UINT24 mos_BOOT(char * filename, char * buffer, INT24 size) {
	FRESULT	fr;
	FIL	   	fil;
	UINT   	br;	
	void * 	dest;
	FSIZE_t fSize;
	
	fr = f_open(&fil, filename, FA_READ);
	if(fr == FR_OK) {
		while(!f_eof(&fil)) {
			f_gets(buffer, size, &fil);
			mos_exec(buffer);
		}
	}
	f_close(&fil);	
	return fr;	
}

// Open a file
// Parameters:
// - filename: Path of file to open
// - mode: File open mode (r, r/w, w, etc) - see FatFS documentation for more details
// Returns:
// - File handle, or 0 if the file cannot be opened
// 
UINT24 mos_FOPEN(char * filename, UINT8 mode) {
	FRESULT fr;
	int		i;
	
	for(i = 0; i < MOS_maxOpenFiles; i++) {
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

// Close file(s)
// Parameters:
// - fh: File handle, or 0 to close all open files
// Returns:
// - File handle passed in function args
//
UINT24 mos_FCLOSE(UINT8 fh) {
	FRESULT fr;
	int 	i;
	
	if(fh > 0 && fh <= MOS_maxOpenFiles) {
		i = fh - 1;
		if(&mosFileObjects[i].free > 0) {
			fr = f_close(&mosFileObjects[i].fileObject);
			mosFileObjects[i].free = 0;
		}
	}
	else {
		for(i = 0; i < MOS_maxOpenFiles; i++) {
			if(mosFileObjects[i].free > 0) {
				fr = f_close(&mosFileObjects[i].fileObject);
				mosFileObjects[i].free = 0;
			}
		}
	}	
	return fh;	
}

// Read a byte from a file
// Parameters:
// - fh: File handle
// Returns:
// - Byte read
//
char	mos_FGETC(UINT8 fh) {
	FRESULT fr;
	UINT	br;
	char	c;

	if(fh > 0 && fh <= MOS_maxOpenFiles) {
		fr = f_read(&mosFileObjects[fh - 1].fileObject, &c, 1, &br); 
		if(fr == FR_OK) {
			return	c;
		}
	}
	return 0;
}

// Write a byte to a file
// Parameters:
// - fh: File handle
// - c: Byte to write
//
void	mos_FPUTC(UINT8 fh, char c) {
	if(fh > 0 && fh <= MOS_maxOpenFiles) {
		f_putc(c, &mosFileObjects[fh - 1].fileObject);
	}
}

// Check whether file is at EOF (end of file)
// Parameters:
// - fh: File handle
// Returns:
// - 1 if EOF, otherwise 0
//
char	mos_FEOF(UINT8 fh) {
	if(fh > 0 && fh <= MOS_maxOpenFiles) {
		if(f_eof(&mosFileObjects[fh - 1].fileObject) != 0) {
			return 1;
		}
	}
	return 0;
}

// Copy an error string to RAM
// Parameters
// - errno: The error number
// - address: Address of the buffer to copy the error code to
// - size: Size of buffer
//
void mos_GETERROR(UINT8 errno, INT24 address, INT24 size) {
	strncpy((char *)address, mos_errors[errno], size - 1);
}

// OSCLI
// Parameters
// - cmd: Address of the command entered
// Returns:
// - MOS error code
//
UINT24 mos_OSCLI(char * cmd) {
	UINT24 fr;
	fr = mos_exec(cmd);
	return fr;
}

