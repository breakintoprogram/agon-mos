;
; Title:	AGON MOS - C Startup Code
; Author:	Copyright (C) 2005 by ZiLOG, Inc.  All Rights Reserved.
; Modified By:	Dean Belfield
; Created:	10/07/2022
; Last Updated:	11/07/2022
;
; Modinfo:
; 11/07/2022:	Added RST_10 code - TX

		XREF __init
		XREF __low_rom
		
		XDEF _reset
		XDEF __default_nmi_handler
		XDEF __default_mi_handler
		XDEF __nvectors
		XDEF _init_default_vectors
		XDEF __init_default_vectors
		XDEF _set_vector
		XDEF __set_vector
		XDEF __2nd_jump_table
		XDEF __1st_jump_table
		XDEF __vector_table

		XREF serial_TX

NVECTORS 	EQU 48			; Number of interrupt vectors

; Save Interrupt State
;
SAVEIMASK	MACRO
		LD	A, I		; Sets parity bit to value of IEF2
		PUSH	AF
		DI			; Disable interrupts while loading table 
		MACEND

; Restore Interrupt State
;
RESTOREIMASK	MACRO
		POP	AF
		JP	PO, $+5		; Parity bit is IEF2
		EI
		MACEND

; Reset and all RST nn's
;
		DEFINE .RESET, SPACE = ROM
		SEGMENT .RESET

_reset:	
_rst0:		DI
		RSMIX
		JP.LIL	__init
		
_rst8:		RET
		DS	7
		
_rst10:		JP	serial_TX
		DS	5

_rst18:		RET
		DS	7
		
_rst20:		RET
		DS	7
		
_rst28:		RET
		DS	7

_rst30:		RET
		DS	7

_rst38:		RET
		DS 	%2D
		
_nmi:		JP.LIL	__default_nmi_handler

; Startup code
		DEFINE .STARTUP, SPACE = ROM
		SEGMENT .STARTUP
		.ASSUME ADL=1

; Number of vectors supported
;
__nvectors:
		DW NVECTORS            ; extern unsigned short _num_vectors;

; Default Non-Maskable Interrupt handler
;
__default_nmi_handler:
		RETN

; Default Maskable Interrupt handler
__default_mi_handler:
		EI
		RETI

; Initialize all potential interrupt vector locations with a known
; default handler.
;
; void _init_default_vectors(void);
;
__init_default_vectors:
_init_default_vectors:
		push af
		SAVEIMASK
		ld hl, __default_mi_handler
		ld a, %C3
		ld (__2nd_jump_table), a       ; place jp opcode
		ld (__2nd_jump_table + 1), hl  ; __default_hndlr
		ld hl, __2nd_jump_table
		ld de, __2nd_jump_table + 4
		ld bc, NVECTORS * 4 - 4
		ldir
		im 2                       ; Interrtup mode 2
		ld a, __vector_table >> 8
		ld i, a                    ; Load interrtup vector base
		RESTOREIMASK
		pop af
		ret

; Installs a user interrupt handler in the 2nd interrupt vector jump table
;
; void * _set_vector(unsigned int vector, void(*handler)(void));
;
__set_vector:
_set_vector:	
		push iy
		ld iy, 0
		add iy, sp                 ; Standard prologue
		push af
		SAVEIMASK
		ld bc, 0                   ; clear bc
		ld b, 2                    ; calculate 2nd jump table offset
		ld c, (iy+6)               ; vector offset
		mlt bc                     ; bc is 2nd jp table offset
		ld hl, __2nd_jump_table
		add hl, bc                 ; hl is location of jp in 2nd jp table
		ld (hl), %C3               ; place jp opcode just in case
		inc hl                     ; hl is jp destination address
		ld bc, (iy+9)              ; bc is isr address
		ld de, (hl)                ; save previous handler
		ld (hl), bc                ; store new isr address
		push de
		pop hl                     ; return previous handler
		RESTOREIMASK
		pop af
		ld sp, iy                  ; standard epilogue
		pop iy
		ret


		DEFINE IVJMPTBL, SPACE = RAM
		SEGMENT IVJMPTBL

; 2nd Interrupt Vector Jump Table
;  - this table must reside in RAM anywhere in the 16M byte range
;  - each 4-byte entry is a jump to an interrupt handler
;
__2nd_jump_table:
		DS NVECTORS * 4


; Interrupt Vector Table
;  - this segment must be aligned on a 256 byte boundary anywhere below
;    the 64K byte boundry
;  - each 2-byte entry is a 2-byte vector address
;
		DEFINE .IVECTS, SPACE = ROM, ALIGN = 100h
		SEGMENT .IVECTS

__vector_table:	
		dw __1st_jump_table + %00
		dw __1st_jump_table + %04
		dw __1st_jump_table + %08
		dw __1st_jump_table + %0c
		dw __1st_jump_table + %10
		dw __1st_jump_table + %14
		dw __1st_jump_table + %18
		dw __1st_jump_table + %1c
		dw __1st_jump_table + %20
		dw __1st_jump_table + %24
		dw __1st_jump_table + %28
		dw __1st_jump_table + %2c
		dw __1st_jump_table + %30
		dw __1st_jump_table + %34
		dw __1st_jump_table + %38
		dw __1st_jump_table + %3c
		dw __1st_jump_table + %40
		dw __1st_jump_table + %44
		dw __1st_jump_table + %48
		dw __1st_jump_table + %4c
		dw __1st_jump_table + %50
		dw __1st_jump_table + %54
		dw __1st_jump_table + %58
		dw __1st_jump_table + %5c
		dw __1st_jump_table + %60
		dw __1st_jump_table + %64
		dw __1st_jump_table + %68
		dw __1st_jump_table + %6c
		dw __1st_jump_table + %70
		dw __1st_jump_table + %74
		dw __1st_jump_table + %78
		dw __1st_jump_table + %7c
		dw __1st_jump_table + %80
		dw __1st_jump_table + %84
		dw __1st_jump_table + %88
		dw __1st_jump_table + %8c
		dw __1st_jump_table + %90
		dw __1st_jump_table + %94
		dw __1st_jump_table + %98
		dw __1st_jump_table + %9c
		dw __1st_jump_table + %a0
		dw __1st_jump_table + %a4
		dw __1st_jump_table + %a8
		dw __1st_jump_table + %ac
		dw __1st_jump_table + %b0
		dw __1st_jump_table + %b4
		dw __1st_jump_table + %b8
		dw __1st_jump_table + %bc

; 1st Interrupt Vector Jump Table
;  - this table must reside in the first 64K bytes of memory
;  - each 4-byte entry is a jump to the 2nd jump table plus offset
;
__1st_jump_table:
		jp __2nd_jump_table + %00
		jp __2nd_jump_table + %04
		jp __2nd_jump_table + %08
		jp __2nd_jump_table + %0c
		jp __2nd_jump_table + %10
		jp __2nd_jump_table + %14
		jp __2nd_jump_table + %18
		jp __2nd_jump_table + %1c
		jp __2nd_jump_table + %20
		jp __2nd_jump_table + %24
		jp __2nd_jump_table + %28
		jp __2nd_jump_table + %2c
		jp __2nd_jump_table + %30
		jp __2nd_jump_table + %34
		jp __2nd_jump_table + %38
		jp __2nd_jump_table + %3c
		jp __2nd_jump_table + %40
		jp __2nd_jump_table + %44
		jp __2nd_jump_table + %48
		jp __2nd_jump_table + %4c
		jp __2nd_jump_table + %50
		jp __2nd_jump_table + %54
		jp __2nd_jump_table + %58
		jp __2nd_jump_table + %5c
		jp __2nd_jump_table + %60
		jp __2nd_jump_table + %64
		jp __2nd_jump_table + %68
		jp __2nd_jump_table + %6c
		jp __2nd_jump_table + %70
		jp __2nd_jump_table + %74
		jp __2nd_jump_table + %78
		jp __2nd_jump_table + %7c
		jp __2nd_jump_table + %80
		jp __2nd_jump_table + %84
		jp __2nd_jump_table + %88
		jp __2nd_jump_table + %8c
		jp __2nd_jump_table + %90
		jp __2nd_jump_table + %94
		jp __2nd_jump_table + %98
		jp __2nd_jump_table + %9c
		jp __2nd_jump_table + %a0
		jp __2nd_jump_table + %a4
		jp __2nd_jump_table + %a8
		jp __2nd_jump_table + %ac
		jp __2nd_jump_table + %b0
		jp __2nd_jump_table + %b4
		jp __2nd_jump_table + %b8
		jp __2nd_jump_table + %bc

		END
