;
; Title:	AGON MOS - RTC
; Author:	Dean Belfield
; Created:	09/03/2023
; Last Updated:	05/06/2023
;
; Modinfo:
; 05/06/2023:	System variable rtc_enable now set

			INCLUDE	"macros.inc"
			INCLUDE	"equs.inc"

			.ASSUME	ADL = 1

			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	__init_rtc
			XDEF	_init_rtc
			XDEF	_millis_handler

			XREF	_rtc_enable
			XREF	_millis

; Initialise the real time clock registers
;
__init_rtc:
_init_rtc:		
			PUSH	AF
			LD	A, 1
			LD	(_rtc_enable), A
			POP	AF
			RET

_millis_handler:	
			DI
			PUSH	AF
			IN0		A,(TMR0_CTL)		; Clear the timer interrupt
			PUSH	BC
			LD		BC, (_millis)		; Increment the delay timer
			INC		BC
			LD		(_millis), BC
			POP		BC
			POP		AF
			EI
			RETI.L