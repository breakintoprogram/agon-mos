/*
 * Title:			AGON MOS - UART code
 * Author:			Dean Belfield
 * Created:			06/07/2022
 * Last Updated:	06/07/2022
 * 
 * Modinfo:
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
	PD_DR = PORTD_DRVAL_DEF ;
	PD_DDR = PORTD_DDRVAL_DEF ;
	#ifdef _EZ80F91
	PD_ALT0 = PORTD_ALT0VAL_DEF ;
	#endif
	PD_ALT1 = PORTD_ALT1VAL_DEF ;
	PD_ALT2 = PORTD_ALT2VAL_DEF ;
	return ;
}

UCHAR open_UART0(UART *pUART) {
	UINT16	br = MASTERCLOCK / (CLOCK_DIVISOR_16 * pUART->baudRate);//! Compute the baudrate generator value;
	UCHAR	pins = PORTPIN_ZERO | PORTPIN_ONE;
	
	SETREG( PD_DDR, pins ) ;										//! Set Port D bits 0, 1 for alternate function.
	RESETREG( PD_ALT1, pins ) ;
	SETREG( PD_ALT2, pins ) ;

	UART0_LCTL |= UART_LCTL_DLAB ;									//! Select DLAB to access baud rate generators
	UART0_BRG_L = (br & 0xFF) ;										//! Load divisor low
	UART0_BRG_H = (CHAR)(( br & 0xFF00 ) >> 8) ;					//! Load divisor high
	UART0_LCTL &= (~UART_LCTL_DLAB) ; 								//! Reset DLAB; dont disturb other bits
	UART0_MCTL = 0x00 ;												//! Bring modem control register to reset value.
	UART0_FCTL = 0x07 ;												//! Disable hardware FIFOs.
	UART0_IER = 0x00 ;												//! Disable all UART interrupts.
	
	SETREG_LCR0(pUART->dataBits, pUART->stopBits, pUART->parity);	//! Set the line status register.
	
	return UART_ERR_NONE ;
}

UCHAR write_UART0(CHAR *pData, int nBytes) {
	int		i;
	for(i = 0; i < nBytes; i++) {
		while ((UART0_LSR & UART_LSR_TEMT) == 0);					//! Wait till the current character is transmitted.
		UART0_THR = pData[i];
	}
	return UART_ERR_NONE;
}

UCHAR read_UART0(CHAR *pData, int *nbytes) {
	UCHAR lsr ;
	UCHAR status = UART_ERR_NONE ;
	int   index = 0 ;

	while( UART_ERR_NONE == status ) {
		lsr = UART0_LSR ;											//! Read the line status.
		
		if( 0 != (lsr & UART_LSR_BREAKINDICATIONERR) ) {			//! Check if there is any Break Indication Error.
			status = UART_ERR_BREAKINDICATIONERR ;					//! Failure code.
		}
		if( 0 != (lsr & UART_LSR_FRAMINGERR) ) {					//! Check if there is any Framing error.
			status = UART_ERR_FRAMINGERR ;							//! Failure code.
		}
		if( 0 != (lsr & UART_LSR_PARITYERR) ) {						//! Check if there is any Parity error.
			status = UART_ERR_PARITYERR ;							//! Failure code.
		}
		if( 0 != (lsr & UART_LSR_OVERRRUNERR) )	{					//! Check if there is any Overrun error.
			status = UART_ERR_OVERRUNERR ;							//! Failure code.
		}
		if( 0 != (lsr & UART_LSR_DATA_READY) ) {					//! See if there is any data byte to be read.
			pData[ index++ ] = UART0_RBR ;							//! Read it from the receive buffer register.
		}
		if( index == (*nbytes) ) {									//! On completion break the while loop.
			break ;
		}
	}
	*nbytes = index ;
	return status ;
	
}

INT putch(INT ich) {
	CHAR ch = ich;
	return write_UART0(&ch, 1) ;									//! Transmit this byte on UART0.;
} 

INT getch(VOID) {
	CHAR ch;
	int nbytes = 1;
	UCHAR status = read_UART0(&ch, &nbytes);
	nbytes = (UINT)ch;
	nbytes = (nbytes & 0x0000FF);
	return (UART_ERR_NONE!=status) ? EOF : nbytes;
}
