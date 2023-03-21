/*
 * Title:			AGON MOS - MOS defines
 * Author:			Dean Belfield
 * Created:			21/03/2023
 * Last Updated:	21/03/2023
 * 
 * Modinfo:
 */

#ifndef MOS_DEFINES_H
#define MOS_DEFINES_H

// VDP specific (for VDU 23,0,n commands)
//
#define VDP_gp			 		0x00
#define VDP_keycode				0x01
#define VDP_cursor				0x02
#define VDP_scrchar				0x03
#define VDP_scrpixel			0x04
#define VDP_audio				0x05
#define VDP_mode				0x06
#define VDP_rtc					0x07
#define VDP_keystate			0x08
#define VDP_logicalcoords		0xC0
#define VDP_terminalmode		0xFF

#endif MOS_DEFINES_H