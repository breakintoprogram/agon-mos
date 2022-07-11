/*
 * Title:			AGON MOS - Timer
 * Author:			Cocoacrumbs
 * Modified by:		Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	19/06/2022
 * 
 * Thank you to @CoCoaCrumbs fo this code https://www.cocoacrumbs.com/
 *
 * Modinfo:
 * 11/07/2022:		Removed unused functions
 */

#include <eZ80.h>

#define TMR2_IVECT 0x0e

static unsigned int volatile g_timer = 0;

extern long SysClkFreq;

void * set_vector(unsigned int vector, void (*hndlr)(void));

void interrupt timer2_isr(void) {
	unsigned char tmp;
	tmp = TMR2_CTL;
    g_timer++;
} 

void init_timer2(int interval) {
	unsigned char tmp;
	unsigned short rr;

	TMR2_CTL = 0x00;

	// Set Timer 2 interrupt vector 
	//
    set_vector(TMR2_IVECT, timer2_isr);

	rr = (unsigned short)(((SysClkFreq / 1000) * interval) / 16);

	TMR2_RR_H = (unsigned char)(rr >> 8);
	TMR2_RR_L = (unsigned char)(rr);

	tmp = TMR2_CTL;
    TMR2_CTL = 0x57;
}

void delayms(int ms) {  
    g_timer = 0;
	while (g_timer < ms); 
} 

