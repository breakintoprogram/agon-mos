/*
 * Title:			AGON MOS - UART code
 * Author:			Dean Belfield
 * Created:			06/07/2022
 * Last Updated:	23/03/2023
 * 
 * Modinfo:
 * 03/08/2022:		Enabled UART0 receive interrupt
 * 08/08/2022:		Enabled UART0 CTS port
 * 22/03/2023:		Moved putch and getch to serial.asm
 * 23/03/2023:		Fixed maths overflow in init_UART0 to work with bigger baud rates
 *
 * NB:
 * The UART is on Port D
 *
 * - 0: RX
 * - 1: TX
 * - 2: RTS (the CTS input of the ESP32
 * - 3: CTS (the RTS output of the ESP32)
 */
 
#include <stddef.h>
#include <stdio.h>
#include <eZ80.h>
#include <defines.h>
#include <gpio.h>

#include "uart.h"
 
// Set the Line Control Register for data, stop and parity bits
//
#define SETREG_LCR0(data, stop, parity) (UART0_LCTL = ((BYTE)(((data)-(BYTE)5)&(BYTE)0x3)|(BYTE)((((stop)-(BYTE)0x1)&(BYTE)0x1)<<(BYTE)0x2)|(BYTE)((parity)<<(BYTE)0x3)))

VOID init_UART0() {
	PD_DR = PORTD_DRVAL_DEF;
	PD_DDR = PORTD_DDRVAL_DEF;
//	#ifdef _EZ80F91
//	PD_ALT0 = PORTD_ALT0VAL_DEF;
//	#endif
	PD_ALT1 = PORTD_ALT1VAL_DEF;
	PD_ALT2 = PORTD_ALT2VAL_DEF;
	return ;
}

// Open UART
// Parameters:
// - pUART: Structure containing the initialisation data
//
UCHAR open_UART0(UART *pUART) {
	UINT32	mc = MASTERCLOCK;										// UART baud rate calculation
	UINT32	cb = CLOCK_DIVISOR_16 * pUART->baudRate;				// split to avoid eZ80 maths overflow error
	UINT32	br = mc / cb;											// with larger baud rate values

	UCHAR	pins = PORTPIN_ZERO | PORTPIN_ONE;						// The transmit and receive pins											
	
	SETREG(PD_DDR, pins);											// Set Port D bits 0, 1 (TX. RX) for alternate function.
	RESETREG(PD_ALT1, pins);
	SETREG(PD_ALT2, pins);
	
	SETREG(PD_DDR, PORTPIN_THREE);									// Set Port D bit 3 (CTS) for input
	RESETREG(PD_ALT1, PORTPIN_THREE);
	RESETREG(PD_ALT2, PORTPIN_THREE);
	
	UART0_LCTL |= UART_LCTL_DLAB ;									// Select DLAB to access baud rate generators
	UART0_BRG_L = (br & 0xFF) ;										// Load divisor low
	UART0_BRG_H = (CHAR)(( br & 0xFF00 ) >> 8) ;					// Load divisor high
	UART0_LCTL &= (~UART_LCTL_DLAB) ; 								// Reset DLAB; dont disturb other bits
	UART0_MCTL = 0x00 ;												// Bring modem control register to reset value.
	UART0_FCTL = 0x07 ;												// Disable hardware FIFOs.
	UART0_IER = UART_IER_RECEIVEINT ;
	
	SETREG_LCR0(pUART->dataBits, pUART->stopBits, pUART->parity);	//! Set the line status register.
	
	return UART_ERR_NONE ;
}
