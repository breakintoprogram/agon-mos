;
; Title:	AGON MOS - Globals
; Author:	Dean Belfield
; Created:	01/08/2022
; Last Updated:	15/03/2023
;
; Modinfo:
; 09/08/2022:	Added sysvars structure, cursorX, cursorY
; 18/08/2022:	Added scrchar, scrpixel, audioChannel, audioSuccess, vdp_protocol_flags
; 18/09/2022:	Added scrwidth, scrheight, scrcols, scrrows
; 23/02/2023:	Added scrcolours, fixed offsets in sysvars comments
; 04/03/2023:	Added scrpixelIndex
; 09/03/2023:	Added vdp_protocol_count, keyascii, keydown; swapped keyascii with keycode, removed timer2
; 15/03/2023:	Added rtc

			INCLUDE	"../src/equs.inc"
			
			XDEF	_sysvars
			
			XDEF 	_keycode
			XDEF	_keymods
			XDEF	_keyascii
			XDEF	_keydown
			XDEF	_keycount
			XDEF	_clock
			XDEF	_cursorX
			XDEF	_cursorY
			XDEF	_scrchar
			XDEF	_scrpixel
			XDEF	_audioChannel
			XDEF	_audioSuccess
			XDEF	_scrwidth
			XDEF	_scrheight
			XDEF	_scrcols
			XDEF	_scrrows
			XDEF	_scrcolours
			XDEF	_scrpixelIndex
			XDEF	_rtc 

			XDEF	_errno
			XDEF 	_coldBoot
			XDEF 	_callSM

			XDEF	_vpd_protocol_flags
			XDEF	_vdp_protocol_state
			XDEF	_vdp_protocol_cmd
			XDEF	_vdp_protocol_len
			XDEF	_vdp_protocol_ptr
			XDEF	_vdp_protocol_data

			SEGMENT BSS		; This section is reset to 0 in cstartup.asm
			
_sysvars:					; Please make sure the sysvar offsets match those in mos_api.inc
;
_clock			DS	4		; + 00h: Clock timer in centiseconds (incremented by 2 every VBLANK)
_vpd_protocol_flags:	DS	1		; + 04h: Flags to indicate completion of VDP commands
_keyascii:		DS	1		; + 05h: ASCII keycode, or 0 if no key is pressed
_keymods:		DS	1		; + 06h: Keycode modifiers
_cursorX:		DS	1		; + 07h: Cursor X position
_cursorY:		DS	1		; + 08h: Cursor Y position
_scrchar		DS	1		; + 09h: Character read from screen
_scrpixel:		DS	3		; + 0Ah: Pixel data read from screen (R,B,G)
_audioChannel:		DS	1		; + 0Dh: Audio channel 
_audioSuccess:		DS	1		; + 0Eh: Audio channel note queued (0 = no, 1 = yes)
_scrwidth:		DS	2		; + 0Fh: Screen width in pixels
_scrheight:		DS	2		; + 11h: Screen height in pixels
_scrcols:		DS	1		; + 13h: Screen columns in characters
_scrrows:		DS	1		; + 14h: Screen rows in characters
_scrcolours:		DS	1		; + 15h: Number of colours displayed
_scrpixelIndex:		DS	1		; + 16h: Index of pixel data read from screen
_keycode:		DS	1		; + 17h: Virtual key code from FabGL
_keydown:		DS	1		; + 18h; Virtual key state from FabGL (0=up, 1=down)
_keycount:		DS	1		; + 19h: Incremented every time a key packet is received
_rtc:			DS	8		; + 1Ah: Real time clock data

_errno:			DS 	3		; extern int _errno
_coldBoot:		DS	1		; extern char _coldBoot
_callSM:		DS	5		; Self-modding code for CALL.IS (HL)


; VDP Protocol Flags
;
; Bit 0: Cursor packet received
; Bit 1: Screen character packet received
; Bit 2: Pixel point packet received
; Bit 3: Audio packet received
; Bit 4: Mode packet received
; Bit 5: Unused
; Bit 6: Unused
; Bit 7: Unused
;
; VDP protocol variables
;
_vdp_protocol_state:	DS	1		; UART state
_vdp_protocol_cmd:	DS	1		; Command
_vdp_protocol_len:	DS	1		; Size of packet data
_vdp_protocol_ptr:	DS	3		; Pointer into data
_vdp_protocol_data:	DS	VDPP_BUFFERLEN
		
			SECTION DATA		; This section is copied to RAM in cstartup.asm

			END