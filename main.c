/*
 * Title:			AGON MOS
 * Author:			Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	11/07/2022
 *
 * Modinfo:
 * 11/07/2022:		Tweaks for Agon Light, Command Line code added
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
#define		MOS_revision 	1
		
static FATFS fs;				// Handle for the file system
static char  cmd[256];			// Array for the command line handler

void wait_ESP32(void) {
	INT ch = 0; 	
	while(ch != 27) {
		ch = getch();
	}	
}

int main(void) {
	UART 	pUART;
/*
	DIR	  	dir;
	FRESULT	fr;
	static 	FILINFO  fno;
	FIL	   	fil;
	BYTE   	buffer;
	UINT   	br;	
	INT 	ch;
*/
	pUART.baudRate = 384000;
	pUART.dataBits = 8;
	pUART.stopBits = 1;
	pUART.parity = PAR_NOPARITY;
	
	init_timer2(1);			// Initialise Timer 2 @ 1ms interval
	init_spi();				// Initialise SPI comms for the SD card interface
	init_UART0();			// Initialise UART0 for the ESP32 interface
	EI();					// Enable the eZ80 interrupts
	open_UART0(&pUART);		// Open the UART 
	f_mount(&fs, "", 1);	// Mount the SD card
	
	wait_ESP32();			// Wait for the ESP32 to finish its bootup
	
	printf("AGON MOS Version %d.%02d\n\r\n\r", MOS_version, MOS_revision);
	
	while(1) {
		mos_input(&cmd, sizeof(cmd));
		mos_exec(&cmd, sizeof(cmd));
	}
/*
	f_mount(&fs, "", 1);
	
	fr = f_opendir(&dir, "/");
	if(fr == FR_OK) {
		for(;;) {
			fr = f_readdir(&dir, &fno);
			if (fr != FR_OK || fno.fname[0] == 0) {
				break;  // Break on error or end of dir
			}
			printf("%s\n\r", fno.fname);
		}
	}
	f_closedir(&dir);
	
	fr = f_open(&fil, "test1.txt", FA_READ);
	if(fr == FR_OK) {
		for(;;) {
			fr = f_read(&fil, &buffer, 1, &br);
			if(br == 0) {
				break;
			}
			putch(buffer);
		}
		printf("\n\r");
	}
	f_close(&fil);
	
	f_unmount("");
	
	while(1) {
		ch = getch();
		if(ch > 0) {
			printf("The key %c %d was pressed\n\r", ch, ch);
		}
	}
*/
	return 0;
}