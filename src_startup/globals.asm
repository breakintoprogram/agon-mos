;
; Title:	AGON MOS - Globals
; Author:	Dean Belfield
; Created:	01/08/2022
; Last Updated:	01/08/2022
;
; Modinfo:
;
			INCLUDE	"../src/equs.inc"
			
			XDEF	_errno
			XDEF 	_coldBoot		
			XDEF 	_keycode
			XDEF	_keymods
			XDEF 	_timer2
			XDEF	_clock
			XDEF 	_callSM
			XDEF	_vdp_protocol_state
			XDEF	_vdp_protocol_cmd
			XDEF	_vdp_protocol_len
			XDEF	_vdp_protocol_ptr
			XDEF	_vdp_protocol_data

			SEGMENT BSS		; This section is reset to 0 in cstartup.asm

_errno:			DS 	3		; extern int _errno
_coldBoot:		DS	1		; extern char _coldBoot
_keycode:		DS	1		; ASCII keycode, or 0 if no key is pressed
_keymods:		DS	1		; Keycode modifiers
_timer2:		DS	3		; Used by delay code, is reset when delayms is called
_clock			DS	4		; Clock timer in centiseconds (incremented by 2 every VBLANK)
_callSM:		DS	5		; Self-modding code for CALL.IS (HL)
	
_vdp_protocol_state:	DS	1		; UART state
_vdp_protocol_cmd:	DS	1
_vdp_protocol_len:	DS	1		; Size of packet data
_vdp_protocol_ptr:	DS	3		; Pointer into data
_vdp_protocol_data:	DS	VDPP_BUFFERLEN		
		
			SECTION DATA		; This section is copied to RAM in cstartup.asm

			END