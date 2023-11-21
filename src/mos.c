/*
 * Title:			AGON MOS - MOS code
 * Author:			Dean Belfield
 * Created:			10/07/2022
 * Last Updated:	11/11/2023
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
 * 12/03/2023:		Renamed keycode to keyascii, keyascii now a BYTE, added mos_cmdTIME, mos_cmdCREDITS, mos_DIR now accepts a path
 * 15/03/2023:		Added mos_cmdCOPY, mos_COPY, mos_GETRTC, aliase for mos_REN, made error messages a bit more user friendly
 * 19/03/2023:		Fixed compilation warnings in mos_cmdTIME
 * 21/03/2023:		Added mos_SETINTVECTOR, uses VDP values from defines.h
 * 26/03/2023:		Fixed SET KEYBOARD command
 * 14/04/2023:		Added fat_EOF
 * 15/04/2023:		Added mos_GETFIL, mos_FREAD, mos_FWRITE, mos_FLSEEK, refactored MOS file commands
 * 30/05/2023:		Fixed bug in mos_parseNumber to detect invalid numeric characters, mos_FGETC now returns EOF flag
 * 08/07/2023:		Added mos_trim function; mos_exec now trims whitespace from input string, various bug fixes
 * 15/09/2023:		Function mos_trim now includes the asterisk character as whitespace
 * 26/09/2023:		Refactored mos_GETRTC and mos_SETRTC
 * 10/11/2023:		Added CONSOLE to mos_cmdSET
 * 11/11/2023:		Added mos_cmdHELP, mos_cmdTYPE, mos_cmdCLS, mos_cmdMOUNT, mos_mount
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "mos.h"
#include "config.h"
#include "mos_editor.h"
#include "uart.h"
#include "clock.h"
#include "ff.h"
#include "strings.h"

extern void *	set_vector(unsigned int vector, void(*handler)(void));	// In vectors16.asm

extern int 		exec16(UINT24 addr, char * params);	// In misc.asm
extern int 		exec24(UINT24 addr, char * params);	// In misc.asm

extern volatile	BYTE keyascii;					// In globals.asm
extern volatile	BYTE vpd_protocol_flags;		// In globals.asm
extern BYTE 	rtc;							// In globals.asm

static FATFS	fs;					// Handle for the file system
static char * 	mos_strtok_ptr;		// Pointer for current position in string tokeniser

extern volatile BYTE history_no;

t_mosFileObject	mosFileObjects[MOS_maxOpenFiles];

// Array of MOS commands and pointer to the C function to run
//
static t_mosCommand mosCommands[] = {
	{ ".", 			&mos_cmdDIR,		HELP_CAT_ARGS,		HELP_CAT,		HELP_DOT_ALIASES },
	{ "DIR",		&mos_cmdDIR,		HELP_CAT_ARGS,		HELP_CAT,		HELP_DIR_ALIASES },
	{ "CAT",		&mos_cmdDIR,		HELP_CAT_ARGS,		HELP_CAT,		HELP_CAT_ALIASES },
	{ "LOAD",		&mos_cmdLOAD,		HELP_LOAD_ARGS,		HELP_LOAD,		NULL },
	{ "SAVE", 		&mos_cmdSAVE,		HELP_SAVE_ARGS,		HELP_SAVE,		NULL },
	{ "DELETE",		&mos_cmdDEL,		HELP_DELETE_ARGS,	HELP_DELETE,	HELP_DELETE_ALIASES	},
	{ "ERASE",		&mos_cmdDEL,		HELP_DELETE_ARGS,	HELP_DELETE,	HELP_ERASE_ALIASES	},
	{ "JMP",		&mos_cmdJMP,		HELP_JMP_ARGS,		HELP_JMP,		NULL },
	{ "RUN", 		&mos_cmdRUN,		HELP_RUN_ARGS,		HELP_RUN,		NULL },
	{ "CD", 		&mos_cmdCD,			HELP_CD_ARGS,		HELP_CD,		NULL },
	{ "RENAME",		&mos_cmdREN,		HELP_RENAME_ARGS,	HELP_RENAME,	HELP_RENAME_ALIASES	},
	{ "MOVE",		&mos_cmdREN,		HELP_RENAME_ARGS,	HELP_RENAME,	HELP_MOVE_ALIASES	},
	{ "MKDIR", 		&mos_cmdMKDIR,		HELP_MKDIR_ARGS,	HELP_MKDIR,		NULL },
	{ "COPY", 		&mos_cmdCOPY,		HELP_COPY_ARGS,		HELP_COPY,		NULL },
	{ "SET",		&mos_cmdSET,		HELP_SET_ARGS,		HELP_SET,		NULL },
	{ "VDU",		&mos_cmdVDU,		HELP_VDU_ARGS,		HELP_VDU,		NULL },
	{ "TIME", 		&mos_cmdTIME,		HELP_TIME_ARGS,		HELP_TIME,		NULL },
	{ "CREDITS",	&mos_cmdCREDITS,	NULL,				HELP_CREDITS,	NULL },
	{ "TYPE",		&mos_cmdTYPE,		HELP_TYPE_ARGS,		HELP_TYPE,		NULL },
	{ "CLS",		&mos_cmdCLS,		NULL,				HELP_CLS,		NULL },
	{ "MOUNT",		&mos_cmdMOUNT,		NULL,				HELP_MOUNT,		NULL },
	{ "HELP",		&mos_cmdHELP,		HELP_HELP_ARGS,		HELP_HELP,		NULL },
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
	"Access denied or directory full",
	"Access denied",
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

#define mos_errors_count (sizeof(mos_errors)/sizeof(char *))

// Output a file error
// Parameters:
// - error: The FatFS error number
//
void mos_error(int error) {
	if(error >= 0 && error < mos_errors_count) {
		printf("\n\r%s\n\r", mos_errors[error]);
	}
}

// Wait for a keycode character from the VPD
// Returns:
// - ASCII keycode
//
BYTE mos_getkey() {
	BYTE ch = 0;
	while(ch == 0) {		// Loop whilst no key pressed
		ch = keyascii;		// Variable keyascii is updated by interrupt
	}
	keyascii = 0;			// Reset keycode to debounce the key
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

// String trim function
// NB: This also includes the asterisk character as whitespace
// Parameters:
// - s: Pointer to the string to trim
// Returns:
// - s: Pointer to the start of the new string
//
char * mos_trim(char * s) {
    char * ptr;

    if(!s) {								// Return NULL if a null string is passed
        return NULL;
	}
    if(!*s) {
        return s;      						// Handle empty string
	}
	while(isspace(*s) || *s == '*') {		// Advance the pointer to the first non-whitespace or asterisk character in the string
		s++;
	}
	ptr = s + strlen(s) - 1;
	while(ptr > s && isspace(*ptr)) {
		ptr--;
	}
	ptr[1] = '\0';
    return s;
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
	if(*e != 0) {
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

	ptr = mos_trim(buffer);
	ptr = mos_strtok(ptr, " ");
	if(ptr != NULL) {
		func = mos_getCommand(ptr);
		if(func != 0) {
			fr = func(ptr);
		}
		else {		
			if(strlen(ptr) > 246) {	// Maximum command length (to prevent buffer overrun)
				fr = 20;
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
	char	*path;

	if(!mos_parseString(NULL, &path)) {
		return mos_DIR(".");
	}
	return mos_DIR(path);
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

// COPY <filename1> <filename2> command
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdCOPY(char *ptr) {
	FRESULT	fr;
	char *  filename1;
	char *	filename2;
	
	if(
		!mos_parseString(NULL, &filename1) ||
		!mos_parseString(NULL, &filename2)
	) {
		return 19; // Bad Parameter
	}
	fr = mos_COPY(filename1, filename2);
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
	if(strcasecmp(command, "KEYBOARD") == 0 && value <= 15) {
		putch(23);
		putch(0);
		putch(VDP_keycode);
		putch(value & 0xFF);
		return 0;
	}
	if(strcasecmp(command, "CONSOLE") == 0 && value <= 1) {
		putch(23);
		putch(0);
		putch(VDP_consolemode);
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
int mos_cmdVDU(char *ptr) {
    char *value_str;
    UINT24 value = 0;
    
    while (mos_parseString(NULL, &value_str)) {
        UINT8 isLong = 0;
        UINT8 base = 10;
        char *endPtr;
        size_t len = strlen(value_str);

        //Strip semicolon notation and set as Long
        if (len > 0 && value_str[len - 1] == ';') {
            value_str[len - 1] = '\0';
            len--;
            isLong = 1;
        }

        // Check for '0x' or '0X' prefix
        if (len > 2 && (value_str[0] == '0' && tolower(value_str[1] == 'x'))) {
            base = 16;
        }
		
        // Check for '&' prefix
        if (value_str[0] == '&') {
            base = 16;
            value_str++;
            len--;
        }
        // Check for 'h' suffix
        if (len > 0 && tolower(value_str[len - 1]) == 'h') {
            value_str[len - 1] = '\0';
            base = 16;
        }

        value = strtol(value_str, &endPtr, base);

        if (*endPtr != '\0' || value > 65535) {
            return 19;
        }
		
        if (value > 255) {
            isLong = 1;
        }

        if (isLong) {
            putch(value & 0xFF); // write LSB
            putch(value >> 8);   // write MSB
        } else {
            putch(value);
        }
    }

    return 0;
}

// TIME
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - 0
//
int mos_cmdTIME(char *ptr) {
	UINT24	yr, mo, da, ho, mi, se;
	char	buffer[64];

	// If there is a first parameter
	//
	if(mos_parseNumber(NULL, &yr)) {
		//
		// Fetch the rest of the parameters
		//
		if(
			!mos_parseNumber(NULL, &mo) ||
			!mos_parseNumber(NULL, &da) ||
			!mos_parseNumber(NULL, &ho) ||
			!mos_parseNumber(NULL, &mi) ||
			!mos_parseNumber(NULL, &se) 
		) {
			return 19;
		}
		buffer[0] = yr - EPOCH_YEAR;
		buffer[1] = mo;
		buffer[2] = da;
		buffer[3] = ho;
		buffer[4] = mi;
		buffer[5] = se;
		mos_SETRTC((UINT24)buffer);
	}
	// Return the new time
	//
	mos_GETRTC((UINT24)buffer);
	printf("%s\n\r", buffer);
	return 0;
}

// CREDITS
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdCREDITS(char *ptr) {
	printf("FabGL 1.0.8 (c) 2019-2022 by Fabrizio Di Vittorio\n\r");
	printf("FatFS R0.14b (c) 2021 ChaN\n\r");
	printf("\n\r");
	return 0;
}

// TYPE <filename>
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int mos_cmdTYPE(char * ptr) {
	FRESULT	fr;
	char *  filename;
	UINT24 	addr;

	if(!mos_parseString(NULL, &filename))
		return 19; // Bad Parameter

	fr = mos_TYPE(filename);
	return fr;
}

// CLS
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int	mos_cmdCLS(char *ptr) {
	putchar(12);
	return 0;
}

// MOUNT
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// - MOS error code
//
int	mos_cmdMOUNT(char *ptr) {
	int fr;

	fr = mos_mount();
	if (fr != FR_OK)
		mos_error(fr);
	return 0;
}

void printCommandInfo(t_mosCommand * cmd) {
	printf("%s", cmd->name);
	if (cmd->args != NULL)
		printf(" %s", cmd->args);
	if (cmd->aliases != NULL)
		printf(" (Aliases: %s)", cmd->aliases);
	printf("\r\n");
	printf("%s\r\n", cmd->help);
}

// HELP
// Parameters:
// - ptr: Pointer to the argument string in the line edit buffer
// Returns:
// -  0: Success
//   20: Failure
//
int mos_cmdHELP(char *ptr) {
	int i;
	int found = 0;
	char *cmd;

	mos_parseString(NULL, &cmd);
	if (cmd != NULL && strcasecmp(cmd, "all") == 0)
		cmd = NULL;

	for (i = 0; i < sizeof(mosCommands) / sizeof(mosCommands[0]); ++i) {
		if (cmd == NULL) 
			printCommandInfo(&mosCommands[i]);
		else
			if (strcasecmp(cmd, mosCommands[i].name) == 0) {
				printCommandInfo(&mosCommands[i]);
				found = 1;
			}
	}

	if (cmd != NULL && !found) {
		return 20;
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
UINT24 mos_LOAD(char * filename, UINT24 address, UINT24 size) {
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
UINT24	mos_SAVE(char * filename, UINT24 address, UINT24 size) {
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

// Display a file from SD card on the screen
// Parameters:
// - filename: Path of file to load
// Returns:
// - FatFS return code
//
UINT24 mos_TYPE(char * filename) {
	FRESULT	fr;
	FIL	fil;
	UINT   	br;
	void * 	dest;
	FSIZE_t fSize;
	char	buf[512];
	int	i;

	fr = f_open(&fil, filename, FA_READ);
	if(fr != FR_OK)
		goto out1;

	while (1) {
		fr = f_read(&fil, (void *)buf, sizeof buf, &br);
		if (br == 0)
			break;
		for (i = 0; i < br; ++i)
			putchar(buf[i]);
	}

	f_close(&fil);
out1:
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
UINT24	mos_DIR(char * path) {
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
	
	fr = f_opendir(&dir, path);
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
			
			printf("%04d/%02d/%02d\t%02d:%02d %c %*lu %s\n\r", yr + 1980, mo, da, hr, mi, fno.fattrib & AM_DIR ? 'D' : ' ', 8, fno.fsize, fno.fname);
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

// Copy file
// Parameters:
// - filename1: Path of file to rename
// - filename2: New filename
// Returns:
// - FatFS return code
// 
UINT24 mos_COPY(char * filename1, char * filename2) {
	FRESULT fr;
	FIL		fsrc, fdst;
	BYTE	buffer[1024];
	UINT	br, bw;

	// Open the file to copy
	//
	fr = f_open(&fsrc, filename1, FA_READ);
	if(fr) {
		return fr;
	}
	// Open the destination file
	//
	fr = f_open(&fdst, filename2, FA_WRITE | FA_CREATE_NEW);
	if(fr) {
		return fr;
	}
	// Copy the file
	//
	while(1) {
        fr = f_read(&fsrc, buffer, sizeof buffer, &br);	// Read a chunk of data from the source file
        if (br == 0) break;								// Error or EOF
        fr = f_write(&fdst, buffer, br, &bw);			// Write it to the destination file
        if (bw < br) break; 							// Error or Disk Full
	}
    f_close(&fsrc);										// Close both files
    f_close(&fdst);

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
UINT24 mos_BOOT(char * filename, char * buffer, UINT24 size) {
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
		if(mosFileObjects[i].free > 0) {
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
// - Byte read in lower 8 bits
// - EOF in upper 8 bits (1 = EOF)
//
UINT24	mos_FGETC(UINT8 fh) {
	FRESULT fr;
	FIL	*	fo;
	UINT	br;
	char	c;

	fo = (FIL *)mos_GETFIL(fh);
	if(fo > 0) {
		fr = f_read(fo, &c, 1, &br); 
		if(fr == FR_OK) {
			return	c | (fat_EOF(fo) << 8);
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
	FIL * fo = (FIL *)mos_GETFIL(fh);

	if(fo > 0) {
		f_putc(c, fo);
	}
}

// Read a block of data into a buffer
// Parameters:
// - fh: File handle
// - buffer: Address to write the data into
// - btr: Number of bytes to read
// Returns:
// - Number of bytes read
//
UINT24	mos_FREAD(UINT8 fh, UINT24 buffer, UINT24 btr) {
	FRESULT fr;
	FIL *	fo = (FIL *)mos_GETFIL(fh);
	UINT	br = 0;

	if(fo > 0) {
		fr = f_read(fo, (const void *)buffer, btr, &br);
		if(fr == FR_OK) {
			return br;
		}
	}
	return 0;
}

// Write a block of data from a buffer
// Parameters:
// - fh: File handle
// - buffer: Address to read the data from
// - btw: Number of bytes to write
// Returns:
// - Number of bytes written
//
UINT24	mos_FWRITE(UINT8 fh, UINT24 buffer, UINT24 btw) {
	FRESULT fr;
	FIL *	fo = (FIL *)mos_GETFIL(fh);
	UINT	bw = 0;

	if(fo > 0) {
		fr = f_write(fo, (const void *)buffer, btw, &bw);
		if(fr == FR_OK) {
			return bw;
		}
	}
	return 0;
}

// Move the read/write pointer in a file
// Parameters:
// - offset: Position of the pointer relative to the start of the file
// Returns:
// - FRESULT
// 
UINT8  	mos_FLSEEK(UINT8 fh, UINT32 offset) {
	FIL * fo = (FIL *)mos_GETFIL(fh);

	if(fo > 0) {
		return f_lseek(fo, offset);
	}
	return FR_INVALID_OBJECT;
}

// Check whether file is at EOF (end of file)
// Parameters:
// - fh: File handle
// Returns:
// - 1 if EOF, otherwise 0
//
UINT8	mos_FEOF(UINT8 fh) {
	FIL * fo = (FIL *)mos_GETFIL(fh);

	if(fo > 0) {
		return fat_EOF(fo);
	}
	return 0;
}

// Copy an error string to RAM
// Parameters:
// - errno: The error number
// - address: Address of the buffer to copy the error code to
// - size: Size of buffer
//
void mos_GETERROR(UINT8 errno, UINT24 address, UINT24 size) {
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

// Get the RTC
// Parameters:
// - address: Pointer to buffer to store time in
// Returns:
// - size of string
//
UINT8 mos_GETRTC(UINT24 address) {
	vdp_time_t t;

	rtc_update();
	rtc_unpack(&rtc, &t);
	rtc_formatDateTime((char *)address, &t);

	return strlen((char *)address);
}

// Set the RTC
// Parameters:
// - address: Pointer to buffer that contains the time data
// Returns:
// - size of string
//
void mos_SETRTC(UINT24 address) {
	BYTE * p = (BYTE *)address;

	putch(23);				// Set the ESP32 time
	putch(0);
	putch(VDP_rtc);
	putch(1);				// 1: Set time (6 byte buffer mode)
	//
	putch(*p++);			// Year
	putch(*p++);			// Month
	putch(*p++);			// Day
	putch(*p++);			// Hour
	putch(*p++);			// Minute
	putch(*p);				// Second
}

// Set an interrupt vector
// Parameters:
// - vector: The interrupt vector to set
// - address: Address of the interrupt handler
// Returns:
// - address: Address of the previous interrupt handler
//
UINT24 mos_SETINTVECTOR(UINT8 vector, UINT24 address) {
	void (* handler)(void) = (void *)address;
	#if DEBUG > 0
	printf("@mos_SETINTVECTOR: %02X,%06X\n\r", vector, address);
	#endif
	return (UINT24)set_vector(vector, handler);
}

// Get a FIL struct from a filehandle
// Parameters:
// - fh: The filehandle (indexed from 1)
// Returns:
// - address of the file structure, or 0 if invalid fh
//
UINT24	mos_GETFIL(UINT8 fh) {
	t_mosFileObject	* mfo;

	if(fh > 0 && fh <= MOS_maxOpenFiles) {
		mfo = &mosFileObjects[fh - 1];
		if(mfo->free > 0) {
			return (UINT24)(&mfo->fileObject);
		}
	}
	return 0;
}

// Check whether file is at EOF (end of file)
// Parameters:
// - fp: Pointer to file structure
// Returns:
// - 1 if EOF, otherwise 0
//
UINT8 fat_EOF(FIL * fp) {
	if(f_eof(fp) != 0) {
		return 1;
	}
	return 0;
}

// (Re-)mount the MicroSD card
// Parameters:
// - None
// Returns:
// - fatfs error code
//
int mos_mount(void) {
	return f_mount(&fs, "", 1);			// Mount the SD card
}

