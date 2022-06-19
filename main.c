/*
 * Title:			AGON MOS
 * Author:			Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	19/06/2022
 *
 * Modinfo:
 */

#include <eZ80.h>

#include <stdio.h>
#include <CTYPE.h>
#include <String.h>
#include <uart.h>
#include <uartdefs.h>

#include "spi.h"

#include "timer.h"
#include "platform.h"

#include "ff.h"

/* ************************************************************************ */

#define min(a, b)               (((a) < (b)) ? (a) : (b))
#define max(a, b)               (((a) > (b)) ? (a) : (b))

/* ************************************************************************ */

int main(void) {
	FATFS fs;   
	DIR	  dir;
	FRESULT fr;
	static FILINFO  fno;
	FIL	   fil;
	BYTE   buffer;
	UINT   br;

	UART pUART;

	pUART.uartMode = POLL;
	pUART.baudRate = 250000;
	pUART.dataBits = 8;
	pUART.stopBits = 1;
	pUART.parity = PAR_NOPARITY;
	pUART.fifoTriggerLevel = FIFO_TRGLVL_NONE;
 	pUART.hwFlowControl = ENABLE_HWFLOW_CONTROL;
	pUART.swFlowControl = DISABLE_SWFLOW_CONTROL;

    timer2_init(1);             // Set to 1 ms interval
    init_hw();
	open_UART0(&pUART);	

	f_mount(&fs, "", 1);
	fr = f_opendir(&dir, "/");
	if(fr == FR_OK) {
		for(;;) {
			fr = f_readdir(&dir, &fno);
			if (fr != FR_OK || fno.fname[0] == 0) {
				break;  // Break on error or end of dir
			}
			printf("%s\n", fno.fname);
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
		printf("\n");
	}
	f_close(&fil);
	
	f_unmount("");

	return 0;
} /* end main */