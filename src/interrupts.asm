;
; Title:	AGON MOS - Interrupt handlers
; Author:	Dean Belfield
; Created:	03/08/2022
; Last Updated:	03/08/2022
;
; Modinfo:

			INCLUDE	"macros.inc"
			INCLUDE	"equs.inc"

			.ASSUME	ADL = 1

			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	_vblank_handler
			XDEF	_timer2_handler
			XDEF	_uart0_handler
			
			XREF	_timer2
			XREF	_clock
			XREF	_vdp_protocol_data
			
			XREF	serial_RX
			XREF	serial_TX
			XREF	mos_api
			XREF	vdp_protocol			
			
; AGON Vertical Blank Interrupt handler
;
_vblank_handler:	DI
			PUSH		AF
			SET_GPIO 	PB_DR, 2		; Need to set this to 2 for the interrupt to work correctly
			PUSH		BC
			PUSH		DE
			PUSH		HL	
			LD 		HL, (_clock)		; Increment the 32-bit clock counter
			LD		BC, 2			; By 2, effectively timing in centiseconds
			ADD		HL, BC
			LD		(_clock), HL
			LD		A, (_clock + 3)
			ADC		A, 0
			LD		(_clock + 3), A			
			POP		HL
			POP		DE
			POP		BC
			POP		AF
			EI	
			RETI.L
			
; AGON Timer 2 Interrupt Handler
;
_timer2_handler:	DI
			PUSH		AF
			IN0  		A,(TMR2_CTL)		; Clear the timer interrupt
			PUSH		BC
			LD		BC, (_timer2)		; Increment the delay timer
			INC		BC
			LD		(_timer2), BC
			POP		BC
			POP		AF
			EI
			RETI.L
			
; AGON UART0 Interrupt Handler
;
_uart0_handler:		DI
			PUSH		AF
			PUSH		BC
			PUSH		DE
			PUSH		HL
			CALL		serial_RX
			LD		C, A		
			LD		HL, _vdp_protocol_data
			CALL		vdp_protocol
			POP		HL
			POP		DE
			POP		BC
			POP		AF
			EI
			RETI.L	