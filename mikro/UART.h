/*
 * UART.h
 *
 * Created: 08.03.2014 13:46:28
 *  Author: Erik
 */ 

#ifndef UART_H_
#define UART_H_

void UART_Init(unsigned int ubrr);
void UART_Transmit(unsigned char data );
unsigned char UART_Receive(void);

#endif  // UART_H_