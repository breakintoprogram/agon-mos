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
 */

#include <eZ80.h>

// Timer 2 interrupt vectors for the various devices
//
#ifdef _EZ80F91
#define TMR2_IVECT 0x5c
#endif

#ifdef _EZ80F93
#define TMR2_IVECT 0x0e
#endif

#ifdef _EZ80F92
#define TMR2_IVECT 0x0e
#endif

#ifdef _EZ80L92
#define TMR2_IVECT 0x0e
#endif

#ifdef _EZ80190
#define TMR2_IVECT 0x0a
#endif

static unsigned int volatile	g_timer		= 0;  	// Used by delayms() & timer2_isr()
static unsigned long			g_tickCount	= 0;	// Used by millis() & timer2_isr()

extern long SysClkFreq;

void * set_vector(unsigned int vector, void (*hndlr)(void));

void interrupt timer2_isr(void) {
    unsigned char tmp;

#ifdef _EZ80F91
    tmp = TMR2_IIR;
#else
    // _EZ80190, _EZ80L92, _EZ80F92, _EZ80F93 
	tmp = TMR2_CTL;
#endif

    g_timer++;
	g_tickCount++;
} 

void timer2_init(int interval) {
    unsigned char tmp;
	unsigned short rr;

	TMR2_CTL = 0x00;

	// set Timer 2 interrupt vector 
	//
    set_vector(TMR2_IVECT, timer2_isr);

	rr = (unsigned short)(((SysClkFreq / 1000) * interval) / 16);
	TMR2_RR_H = (unsigned char)(rr >> 8);
	TMR2_RR_L = (unsigned char)(rr);

#ifdef _EZ80190
	tmp = TMR2_CTL;
    TMR2_CTL = 0x5f;
#endif

#ifdef _EZ80F91
    tmp = TMR2_IIR;
    TMR2_CTL = 0x0F;
   	TMR2_IER = 0x01;
#endif

#ifdef _EZ80L92
	tmp = TMR2_CTL;
    TMR2_CTL = 0x57;
#endif

#ifdef _EZ80F92
	tmp = TMR2_CTL;
    TMR2_CTL = 0x57;
#endif

#ifdef _EZ80F93
	tmp = TMR2_CTL;
    TMR2_CTL = 0x57;
#endif
} /* end timer2_init */

void delayms(int ms)
{  
    g_timer = 0;

	while (g_timer < ms); 
} 

long millis(void) {
	return g_tickCount;
} 

void tenMicroSeconds(void) {
	// Calling this function together with either digitalWriteClock/digitalWriteData
	// takes 10 uSeconds on a eZ80/18.432MHz/1WaitStateFlash
	//
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
}

void sixtyMicroSeconds(void) {
	// Calling this function together with either digitalWriteClock/digitalWriteData
	// takes 60 uSeconds on a eZ80/18.432MHz/1WaitStateFlash
	//
    short   i;
    
	for (i = 0; i < 6; i++) {
		asm("NOP");
	} 
} 
