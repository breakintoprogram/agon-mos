/*
 * Title:			AGON MOS - Timer
 * Author:			Cocoacrumbs
 * Modified by:		Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	24/07/2022
 * 
 * Thank you to @CoCoaCrumbs fo this code https://www.cocoacrumbs.com/
 *
 * Modinfo:
 * 11/07/2022:		Removed unused functions
 * 24/07/2022:		Moved interrupt handler to vectors16.asm and initialisation to main
 */

#include <eZ80.h>
#include <defines.h>

extern long SysClkFreq;
extern volatile	UINT24 timer2;

void init_timer2(int interval) {
	unsigned char tmp;
	unsigned short rr;
	
	timer2 = 0;

	TMR2_CTL = 0x00;

	rr = (unsigned short)(((SysClkFreq / 1000) * interval) / 16);

	TMR2_RR_H = (unsigned char)(rr >> 8);
	TMR2_RR_L = (unsigned char)(rr);

	tmp = TMR2_CTL;
    TMR2_CTL = 0x57;
}

void delayms(int ms) {  
    timer2 = 0;
	while (timer2  < ms); 
} 

