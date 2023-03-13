/*
 * Title:			AGON MOS - Timer
 * Author:			Cocoacrumbs
 * Modified by:		Dean Belfield
 * Created:			19/06/2022
 * Last Updated:	09/03/2023
 * 
 * Modinfo:
 * 11/07/2022:		Removed unused functions
 * 09/03/2023:      Refactored
 */

#ifndef TIMER_H
#define TIMER_H

unsigned short  init_timer0(int interval, int clkdiv, unsigned char ctrlbits);
void            enable_timer0(unsigned char enable);
unsigned short  get_timer0();

void            wait_timer0();  // In misc.asm

#endif TIMER_H