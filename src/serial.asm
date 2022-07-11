;
; Title:	AGON MOS - UART code
; Author:	Dean Belfield
; Created:	11/07/2022
; Last Updated:	11/07/2022
;
; Modinfo:

			.ASSUME	ADL = 0
			
			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	serial_TX
			XDEF	serial_RX
			XDEF	serial_RX_WAIT
				
PORT			EQU	%C0		; UART0
				
REG_RBR:		EQU	PORT+0		; Receive buffer
REG_THR:		EQU	PORT+0		; Transmitter holding
REG_DLL:		EQU	PORT+0		; Divisor latch low
REG_IER:		EQU	PORT+1		; Interrupt enable
REG_DLH:		EQU	PORT+1		; Divisor latch high
REG_IIR:		EQU	PORT+2		; Interrupt identification
REG_FCT			EQU	PORT+2;		; Flow control
REG_LCR:		EQU	PORT+3		; Line control
REG_MCR:		EQU	PORT+4		; Modem control
REG_LSR:		EQU	PORT+5		; Line status
REG_MSR:		EQU	PORT+6		; Modem status

REG_SCR:		EQU 	PORT+7		; Scratch

TX_WAIT			EQU	2048		; Count before a TX times out

UART_LSR_ERR		EQU 	%80		; Error
UART_LSR_ETX		EQU 	%40		; Transmit empty
UART_LSR_ETH		EQU	%20		; Transmit holding register empty
UART_LSR_RDY		EQU	%01		; Data ready

; Write a character to the UART
; A: Data to write
; Returns:
; F = C if written
; F = NC if timed out
;
serial_TX:		PUSH		BC		; Stack BC
			PUSH		AF 		; Stack AF
			LD		BC,TX_WAIT	; Set CB to the transmit timeout
serial_TX1:		IN0		A,(REG_LSR)	; Get the line status register
			AND 		UART_LSR_ETX	; Check for TX empty
			JR		NZ, serial_TX2	; If set, then TX is empty, goto transmit
			DEC		BC
			LD		A, B
			OR		C
			JR		NZ, serial_TX1
			POP		AF		; We've timed out at this point so
			POP		BC		; Restore the stack
			OR		A		; Clear the carry flag and preserve A
			RET	
serial_TX2:		POP		AF		; Good to send at this point, so
			OUT0		(REG_THR),A	; Write the character to the UART transmit buffer
			POP		BC		; Restore BC
			SCF				; Set the carry flag
			RET 

; As RX, but wil wait until a character is received
; A: Data read
;
serial_RX_WAIT:		CALL 		serial_RX
			JR		NC,serial_RX_WAIT 
			RET 

; Read a character from the UART
; A: Data read
; Returns:
; F = C if character read
; F = NC if no character read
;
serial_RX:		IN0		A,(REG_LSR)	; Get the line status register
			AND 		UART_LSR_RDY	; Check for characters in buffer
			RET		Z		; Just ret (with carry clear) if no characters
			IN0		A,(REG_RBR)	; Read the character from the UART receive buffer
			SCF 				; Set the carry flag
			RET
				