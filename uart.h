/*
 * uart.h
 *
 *  Created on: Oct 2, 2013
 *      Author: mark
 */

#ifndef UART_H_
#define UART_H_

void UART_init(void);
void UART_Off(void);

unsigned char UART_CharAvalible(void);
unsigned char UART_Receive(void);
unsigned char  UART_TransmitReady(void);
void UART_Transmit(unsigned char c);



#endif /* UART_H_ */
