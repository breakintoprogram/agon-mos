;
; Title:	AGON MOS - Miscellaneous helper functions
; Author:	Dean Belfield
; Created:	24/07/2022
; Last Updated:	24/07/2022
;
; Modinfo:

			INCLUDE	"macros.inc"
			INCLUDE	"equs.inc"

			.ASSUME	ADL = 1

			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
							
			XDEF	SWITCH_A
			
			XDEF	__exec16
			XDEF	_exec16
			
			XREF	_callSM
			
; Switch on A - lookup table immediately after call
;  A: Index into lookup table
;
SWITCH_A:		EX	(SP), HL		; Swap HL with the contents of the top of the stack
			ADD	A, A			; Multiply A by two
			ADD8U_HL 			; Add to HL (macro)
			LD	A, (HL)			; follow the call. Fetch an address from the
			INC	HL 			; table.
			LD	H, (HL)
			LD	L, A
			EX	(SP), HL		; Swap this new address back, restores HL
			RET				; Return program control to this new address			

; Execute a program in RAM
; void * _exec16(unsigned long address)
; Params:
; - address: The 24-bit address to call
;
; This function will call the 24 bit address and switch the eZ80 into Z80 mode (ADL=0)
; The called function must do a RET.LIS (49h, C9h) and take care of preserving registers
;
__exec16:
_exec16:		PUSH 	IY
			LD	IY, 0
			ADD	IY, SP		; Standard prologue
			PUSH 	AF		; Stack any registers being used
			PUSH	DE
			PUSH	HL
			LD	A, MB
			PUSH	AF
			LD	DE, (IY+6)	; Get the address
			LD	A, (IY+8)	; And the high byte for the code segment
			LD	MB, A		; Set the MBASE register		
;
; Write out a short subroutine "CALL.IS (DE): RET" to RAM
;
			LD	IY,_callSM	; Storage for the self modified routine
			LD	(IY + 0), 49h	; CALL.IS llhh
			LD	(IY + 1), CDh
			LD	(IY + 2), E
			LD	(IY + 3), D
			LD	(IY + 4), C9h	; RET		
			CALL	_callSM		; Call the subroutine
			
			POP	AF		; Restore the MBASE register
			LD	MB, A
			POP	HL		; Balance the stack
			POP	DE
			POP 	AF
			LD	SP, IY          ; Standard epilogue
			POP	IY
			RET	