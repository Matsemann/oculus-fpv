/*
 * USART.c
 *
 * Created: 04.09.2013 11:32:45
 *  Author: erik
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 7372800
#define FOSC 7372800

void UART_Init( unsigned int ubrr )
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3<<UCSZ00);
}
 UART_Transmit(unsigned char data ){
	while( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}
unsigned char UART_Receive(void){
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

