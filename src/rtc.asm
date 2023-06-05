;
; Title:	AGON MOS - RTC
; Author:	Dean Belfield
; Created:	09/03/2023
; Last Updated:	09/03/2023
;
; Modinfo:

			INCLUDE	"macros.inc"
			INCLUDE	"equs.inc"

			.ASSUME	ADL = 1

			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XREF _rtc_enable
							
			XDEF	__init_rtc
			XDEF	_init_rtc
			
; Initialise the real time clock registers
;
__init_rtc:
_init_rtc:		
			PUSH AF
			LD	A, 1
			LD (_rtc_enable), A
			POP AF
			RET

