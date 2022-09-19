/*
 * Title:			AGON MOS - MOS line editor
 * Author:			Dean Belfield
 * Created:			18/09/2022
 * Last Updated:	18/09/2022
 * 
 * Modinfo:
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mos_editor.h"
#include "mos.h"
#include "uart.h"

extern volatile BYTE vpd_protocol_flags;

extern BYTE cursorX;
extern BYTE cursorY;
extern BYTE scrcols;

// Get the current cursor position from the VPD
//
void getCursorPos() {
	vpd_protocol_flags &= 0xFE;					// Clear the semaphore flag
	putch(23);									// Request the cursor position
	putch(0);
	putch(2);
	while((vpd_protocol_flags & 0x01) == 0);	// Wait until the semaphore has been set
}

// Set the cursor position; eequivalent to TAB(x, y)
// Parameters:
// - x: Cursor X position
// - y: Cursor Y position
// 
void setCursorPos(BYTE x, BYTE y) {
	putch(31);
	putch(x);
	putch(y);
}

// Get the current screen dimensions from the VDU
//
void getModeInformation() {
	vpd_protocol_flags &= 0xEF;					// Clear the semaphore flag
	putch(23);
	putch(0);
	putch(6);
	while((vpd_protocol_flags & 0x10) == 0);	// Wait until the semaphore has been set
}

// Insert a character in the input string
// Parameters:
// - buffer: Pointer to the line edit buffer
// - c: Character to insert
// - insertPos: Position in the input string to insert the character
// - len: Length of the input string (before the character is inserted)
// - limit: Max number of characters to insert
// Returns:
// - true if the character was inserted, otherwise false
//
BOOL insertCharacter(char *buffer, char c, int insertPos, int len, int limit) {
	int	i;
	
	if(len < limit) {
		putch(c);
		getCursorPos();
		for(i = len; i >= insertPos; i--) {
			buffer[i+1] = buffer[i];
		}
		buffer[insertPos] = c;
		for(i = insertPos + 1; i <= len; i++) {
			putch(buffer[i]);
		}
		setCursorPos(cursorX, cursorY);
		return 1;
	}	
	return 0;
}

// Remove a character from the input string
// Parameters:
// - buffer: Pointer to the line edit buffer
// - c: The backspace character to print (typically 0x7F)
// - insertPos: Position in the input string of the character to be deleted
// - len: Length of the input string before the character is deleted
// Returns:
// - true if the character was deleted, otherwise false
//
BOOL deleteCharacter(char *buffer, char c, int insertPos, int len) {
	int	i;
	if(insertPos > 0) {
		if(cursorX > 0) {
			getCursorPos();
			putch(c);
			for(i = insertPos - 1; i < len; i++) {
				BYTE b = buffer[i+1];
				buffer[i] = b;
				putch(b ? b : ' ');
			}
			setCursorPos(cursorX - 1, cursorY);
			return 1;
		}
	}	
	return 0;
}

// The main line edit function
// Parameters:
// - buffer: Pointer to the line edit buffer
// - bufferLength: Size of the buffer in bytes
// Returns:
// - The exit key pressed (ESC or CR)
//
UINT24 mos_EDITLINE(char * buffer, int bufferLength) {
	char key = 0;
	int	 insertPos = 0;
	int  len = 0;
	int  limit = bufferLength - 1;

	getModeInformation();
	
	buffer[0] = 0;	
	
	while(key != 13 && key != 27) {
		len = strlen(buffer);
		key = mos_getkey();
		if(key > 0) {
			if(key >= 32 && key <= 126) {
				if(insertCharacter(buffer, key, insertPos, len, limit)) {
					insertPos++;
				}
			}
			else {				
				switch(key) {
					case 0x08:	// Cursor Left
						if(insertPos > 0) {
							getCursorPos();
							if(cursorX > 0) {
								putch(0x08);
								insertPos--;
							}
						}
						break;
					case 0x15:	// Cursor Right
						if(insertPos < len) {
							getCursorPos();
							if(cursorX < (scrcols - 1)) {
								putch(0x09);
								insertPos++;
							}
						}
						break;
					case 0x0A:	// Cursor Down
						if(insertPos <= (len - scrcols)) {
							putch(0x0A);
							insertPos += scrcols;
						}
						break;
					case 0x0B:	// Cursor Up
						if(insertPos >= scrcols) {
							putch(0x0B);
							insertPos -= scrcols;
						}
						break;
					case 0x7F:	// Backspace
						if(deleteCharacter(buffer, key, insertPos, len)) {
							insertPos--;
						}
						break;
				}					
			}
		}		
	}
	len-=insertPos;					// Now just need to cursor to end of line; get # of characters to cursor

	while(len >= scrcols) {			// First cursor down if possible
		putch(0x0A);
		len-=scrcols;
	}
	while(len-- > 0) putch(0x09);	// Then cursor right for the remainder
	
	return key;						// Finally return the keycode (ESC or CR)
}