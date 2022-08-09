;
; Title:	AGON MOS - Globals
; Author:	Dean Belfield
; Created:	01/08/2022
; Last Updated:	09/08/2022
;
; Modinfo:
; 09/08/2022:	Added sysvars structure, cursorX, cursorY

			INCLUDE	"../src/equs.inc"
			
			XDEF	_sysvars
			
			XDEF 	_keycode
			XDEF	_keymods
			XDEF	_clock
			XDEF	_cursorX
			XDEF	_cursorY

			XDEF	_errno
			XDEF 	_coldBoot
			XDEF 	_timer2
			XDEF 	_callSM

			XDEF	_vdp_protocol_state
			XDEF	_vdp_protocol_cmd
			XDEF	_vdp_protocol_len
			XDEF	_vdp_protocol_ptr
			XDEF	_vdp_protocol_data

			SEGMENT BSS		; This section is reset to 0 in cstartup.asm
			
_sysvars:					; Please make sure the sysvar offsets match those in mos_api.inc
;
_clock			DS	4		; + 00h: Clock timer in centiseconds (incremented by 2 every VBLANK)
_keycode:		DS	1		; + 04h: ASCII keycode, or 0 if no key is pressed
_keymods:		DS	1		; + 05h: Keycode modifiers
_cursorX:		DS	1		; + 06h: Cursor X position
_cursorY:		DS	1		; + 07h: Cursor Y position

_errno:			DS 	3		; extern int _errno
_coldBoot:		DS	1		; extern char _coldBoot
_timer2:		DS	3		; Used by delay code, is reset when delayms is called
_callSM:		DS	5		; Self-modding code for CALL.IS (HL)
	
_vdp_protocol_state:	DS	1		; UART state
_vdp_protocol_cmd:	DS	1
_vdp_protocol_len:	DS	1		; Size of packet data
_vdp_protocol_ptr:	DS	3		; Pointer into data
_vdp_protocol_data:	DS	VDPP_BUFFERLEN		
		
			SECTION DATA		; This section is copied to RAM in cstartup.asm

			END