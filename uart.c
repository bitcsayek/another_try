/*
 * uart.c
 *
 *  Created on: Oct 2, 2013
 *      Author: mark
 */


#include "hwdef.h"
#include "parser.h"


void UART_init(void)
{
	P2SEL |= 0x0C;
	P2DIR &= 0x04;

	UCA2CTL1 |= UCSWRST | UCSSEL_2;

	UCA2CTL0 = 0;

    UCA2MCTLW = UCBRS1;	// 2

	UCA2BR0 = 109;	// 9600 baud
    UCA2BR1 = 0x00;

    								// use smclock ~ 1MHz
    UCA2CTL1 = UCBRKIE | UCSSEL_2;	// change last. release uart from reset
}

void UART_Off(void)
{
	UCA2CTL1 = UCSWRST | UCSSEL_2;
	P2DIR |=  0x0C;
	P2SEL &= ~0x0C;
	P2OUT &= ~0x0C;

}



void UART_Receive(void)
{
	if ( rxIndex >= RX_BUFF_SIZE )
		rxIndex = 0;	// shouldn't happen.

	rxBuff[rxIndex++] = UCA2RXBUF;

	if ( rxIndex >= RX_BUFF_SIZE )
		rxIndex = 0;	// shouldn't happen.

	rxBuff[rxIndex] = 0;	// trailing null.

}

unsigned char  UART_TransmitReady(void)
{
	if ( UCA2IFG & UCTXIFG)
		return TRUE;
	else
		return FALSE;


}


void UART_Transmit(unsigned char c)
{
	while ( !( UCA2IFG & UCTXIFG) )
		;

	UCA2TXBUF = c;

}


int UART_status(void)
{
	return UCA2STATW;
}




