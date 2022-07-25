/*
 * Title:			AGON MOS
 * Author:			Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	25/07/2022
 *
 * Modinfo:
 * 11/07/2022:		Version 0.01: Tweaks for Agon Light, Command Line code added
 * 13/07/2022:		Version 0.02
 * 15/07/2022:		Version 0.03: Warm boot support, VBLANK interrupt
 * 25/07/2022:		Version 0.04; Tweaks to initialisation and interrupts
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
#define		MOS_revision 	4

extern void *	set_vector(unsigned int vector, void(*handler)(void));

extern void 	vblank_handler(void);
extern void 	timer2_handler(void);

extern char coldBoot;				// 1 = cold boot, 0 = warm boot
extern volatile	char keycode;		// Keycode 		

static FATFS 	fs;					// Handle for the file system
static char  	cmd[256];			// Array for the command line handler

// Wait for the ESP32 to respond with an ESC character to signify it is ready
// Parameters: None
// Returns: None
//
void wait_ESP32(void) {
	char ch = 0; 	
	while(ch != 27) {
		ch = getch();
	}	
}

// Initialise the interrupts
//
void init_interrupts(void) {
	set_vector(PORTB1_IVECT, vblank_handler); 	// 0x32
	set_vector(PRT2_IVECT, timer2_handler);		// 0x0E
}

// The main loop
//
int main(void) {
	UART 	pUART;

	pUART.baudRate = 384000;
	pUART.dataBits = 8;
	pUART.stopBits = 1;
	pUART.parity = PAR_NOPARITY;

	DI();					// Ensure interrupts are disabled before we do anything
	init_interrupts();		// Initialise the interrupt vectors
	init_timer2(1);			// Initialise Timer 2 @ 1ms interval
	init_spi();				// Initialise SPI comms for the SD card interface
	init_UART0();			// Initialise UART0 for the ESP32 interface
	open_UART0(&pUART);		// Open the UART 
	if(coldBoot > 0) {		// If a cold boot has been detected
		wait_ESP32();		// Wait for the ESP32 to finish its bootup
	}
	EI();					// Enable the interrupts now
	
	f_mount(&fs, "", 1);	// Mount the SD card

	printf("AGON MOS Version %d.%02d\n\r\n\r", MOS_version, MOS_revision);
	
	while(1) {
		mos_input(&cmd, sizeof(cmd));
		mos_exec(&cmd, sizeof(cmd));
	}

	return 0;
}