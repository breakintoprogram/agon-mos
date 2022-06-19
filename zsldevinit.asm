;  zsldevinit.asm
;  Implementation file for opening peripheral devices.
; 
;  This file contains implementation for opening (by calling intialize routines)
;  peripheral devices required by ZiLOG Standard Library implementations for eZ80
;  and eZ80 Acclaim! series of microprocessors and microcontrollers.
;
;  Copyright (C) 1999-2004 by  ZiLOG, Inc.
;  All Rights Reserved.
;

	include "intvect.inc"

	segment	CODE
	.assume adl=1
	
	XDEF _open_periphdevice
	XDEF __open_periphdevice

_open_periphdevice:
__open_periphdevice:

.ifdef ZSL_DEVICE_UART0
	.define ZSL_UART_USED
.endif

.ifdef ZSL_DEVICE_UART1
.ifndef ZSL_UART_USED
	.define ZSL_UART_USED
.endif
.endif

.ifdef ZSL_DEVICE_PORTA
	XREF _open_PortA

	ld bc, 0
	push bc											; pass a null pointer.
    call _open_PortA								; initialize Port A.
	pop bc
endif

.ifdef ZSL_DEVICE_PORTB
	XREF _open_PortB

	ld bc, 0
	push bc											; pass a null pointer.
    call _open_PortB								; initialize Port B.
	pop bc
endif

.ifdef ZSL_DEVICE_PORTC
	XREF _open_PortC

	ld bc, 0
	push bc											; pass a null pointer.
    call _open_PortC								; initialize Port C.
	pop bc
endif

.ifdef ZSL_DEVICE_PORTD
	XREF _open_PortD

	ld bc, 0
	push bc											; pass a null pointer.
    call _open_PortD								; initialize Port D.
	pop bc
endif



.ifdef ZSL_UART_USED
	XREF _zsl_g_clock_xdefine
	XREF __lcmpu

	ld	hl,#(_zsl_g_clock_xdefine & %ffffff)		; load the symbol value in HL
	ld	e,#(_zsl_g_clock_xdefine >> 24)				; load the highest byte in E
	ld	bc,0
	xor	a,a
	call __lcmpu									; check if it is zero
	jr	z,_skip										; skip if it zero

	ld hl, #(_zsl_g_clock_xdefine & %ffffff)		; load symbol value in g_clock0\1 variables.
	ld a, #(_zsl_g_clock_xdefine >> 24)
.endif

.ifdef ZSL_DEVICE_UART0
	XREF _g_clock0

	ld (_g_clock0), hl
	ld (_g_clock0 + 3), a
.endif
.ifdef ZSL_DEVICE_UART1
	XREF _g_clock1

	ld (_g_clock1), hl
	ld (_g_clock1 + 3), a
.endif
_skip:


.ifdef ZSL_DEVINIT
	XREF _init_default_vectors

	call _init_default_vectors						; Install the vector table with default ISR handlers.
.endif


.ifdef ZSL_DEVICE_UART0
	XREF _open_UART0
	XREF _isr_uart0

.ifdef ZSL_DEVINIT
	XREF _set_vector
	ld bc, _isr_uart0								; Install ISR for UART0
	push bc
	ld bc, UART0_IVECT
	push bc
	call _set_vector
	pop bc
	pop bc
.endif

	ld bc, 0
	push bc											; pass a null pointer.
	call _open_UART0								; initialize UART0.
	pop bc
endif



.ifdef ZSL_DEVICE_UART1
	XREF _open_UART1
	XREF _isr_uart1

.ifdef ZSL_DEVINIT
	XREF _set_vector
	ld bc, _isr_uart1								; Install ISR for UART1
	push bc
	ld bc, UART1_IVECT
	push bc
	call _set_vector
	pop bc
	pop bc
.endif

	ld bc, 0
	push bc											; pass a null pointer.
	call _open_UART1								; initialize UART1.
	pop bc
endif


	ret


	XDEF _close_periphdevice
	XDEF __close_periphdevice

_close_periphdevice:
__close_periphdevice:



.ifdef ZSL_DEVICE_PORTA
	XREF _close_PortA

    call _close_PortA								; close Port A.
endif

.ifdef ZSL_DEVICE_PORTB
	XREF _close_PortB

    call _close_PortB								; close Port B.
endif

.ifdef ZSL_DEVICE_PORTC
	XREF _close_PortC

	call _close_PortC								; close Port C.
endif

.ifdef ZSL_DEVICE_PORTD
	XREF _close_PortD

    call _close_PortD								; close Port D.
endif

.ifdef ZSL_DEVICE_UART0
	XREF _close_UART0

	call _close_UART0								; close UART0.
endif

.ifdef ZSL_DEVICE_UART1
	XREF _close_UART1

	call _close_UART1								; close UART1.
endif

	ret

;---------------------------------------------------------------------------------------
	segment data
BUFF_SIZE .equ 64									; default software FIFO buff size value.

.ifdef ZSL_UART_USED
	XDEF _g_fifosize

_g_fifosize:
	.trio BUFF_SIZE									; Maximum size of the FIFO	
.endif

	segment bss
.ifdef ZSL_DEVICE_UART0
	XDEF _g_RxBuffer_UART0
	XDEF _g_TxBuffer_UART0

_g_RxBuffer_UART0:
	ds BUFF_SIZE									; Allocate space for receive FIFO.

_g_TxBuffer_UART0:
	ds BUFF_SIZE									; Allocate space for transmit FIFO.

.endif
	
.ifdef ZSL_DEVICE_UART1
	XDEF _g_RxBuffer_UART1
	XDEF _g_TxBuffer_UART1

_g_RxBuffer_UART1:
	ds BUFF_SIZE									; Allocate space for receive FIFO.

_g_TxBuffer_UART1:
	ds BUFF_SIZE									; Allocate space for transmit FIFO.
.endif
