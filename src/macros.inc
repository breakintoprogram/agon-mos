;
; Title:	AGON MOS - Useful Macros
; Author:	Dean Belfield
; Created:	15/07/2022
; Last Updated:	15/07/2022
;
; Modinfo:

ADD8U_HL:		MACRO 
			ADD	A, L 
			LD	L, A 
			ADC	A, H
			SUB	L
			LD	H, A 
			ENDMACRO 

SET_GPIO:		MACRO	REG, VAL
			IN0	A,(REG)
			OR	VAL
			OUT0	(REG),A
			ENDMACRO
				
RES_GPIO:		MACRO	REG, VAL
			PUSH	BC
			LD	A, VAL
			CPL
			LD	C, A
			IN0	A,(REG)
			AND	C
			OUT0	(REG),A
			POP	BC
			ENDMACRO
			