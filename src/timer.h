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

#ifndef TIMER_H
#define TIMER_H

void timer2_init(int interval);
void delayms(int ms);

long millis(void);

void tenMicroSeconds(void);
void sixtyMicroSeconds(void);

#endif TIMER_H