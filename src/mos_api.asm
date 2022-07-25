;
; Title:	AGON MOS - API code
; Author:	Dean Belfield
; Created:	24/07/2022
; Last Updated:	24/07/2022
;
; Modinfo:

			.ASSUME	ADL = 1
			
			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	mos_api
			
			XREF	SWITCH_A
			XREF	_keycode


; Call a MOS API function
; A: function to call
;
mos_api:		CALL	SWITCH_A
			DW	mos_api_getkey

; Test API to get keycode
;
mos_api_getkey:		LD	A, (_keycode)
			RET.L

				