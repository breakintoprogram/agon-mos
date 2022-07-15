/*
 * Title:			AGON MOS
 * Author:			Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	15/07/2022
 *
 * Modinfo:
 * 11/07/2022:		Version 0.01: Tweaks for Agon Light, Command Line code added
 * 13/07/2022:		Version 0.02
 * 15/07/2022:		Version 0.03: Warm boot support, VBLANK interrupt
 */

#include <eZ80.h>
#include <defines.h>
#include <stdio.h>
#include <CTYPE.h>
#include <String.h>

#include "uart.h"
#include "spi.h"
#include "timer.h"
#include "ff.h"
#include "mos.h"

#define		MOS_version		0
#define		MOS_revision 	3

extern void *	set_vector(unsigned int vector, void(*handler)(void));
extern void 	vblank_handler(void);

extern char  	coldBoot;			// 1 = cold boot, 0 = warm boot
extern char  	keycode;			// Keycode 		

static FATFS 	fs;					// Handle for the file system
static char  	cmd[256];			// Array for the command line handler

// Wait for the ESP32 to respond with an ESC character to signify it is ready
// Parameters: None
// Returns: None
//
void wait_ESP32(void) {
	INT ch = 0; 	
	while(ch != 27) {
//		ch = getch();
		ch = keycode;
	}	
}

// Initialise the interrupts
//
void init_interrupts(void) {
	DI();
	set_vector(PORTB1_IVECT, &vblank_handler);
	EI();
}

// The main loop
//
int main(void) {
	UART 	pUART;

	pUART.baudRate = 384000;
	pUART.dataBits = 8;
	pUART.stopBits = 1;
	pUART.parity = PAR_NOPARITY;
	
	init_timer2(1);			// Initialise Timer 2 @ 1ms interval
	init_spi();				// Initialise SPI comms for the SD card interface
	init_UART0();			// Initialise UART0 for the ESP32 interface
	open_UART0(&pUART);		// Open the UART 
	init_interrupts();		// Initialise the interrupt vectors
	
	f_mount(&fs, "", 1);	// Mount the SD card
	
	if(coldBoot > 0) {		// If a cold boot has been detected
		wait_ESP32();		// Wait for the ESP32 to finish its bootup
	}

	printf("AGON MOS Version %d.%02d\n\r\n\r", MOS_version, MOS_revision);
	
	while(1) {
		mos_input(&cmd, sizeof(cmd));
		mos_exec(&cmd, sizeof(cmd));
	}

	return 0;
}