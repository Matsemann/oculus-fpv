/*
 * GccApplication1.c
 *
 * Created: 19.02.2014 20:21:01
 *  Author: Erik
 */ 

#define F_CPU 7372800
#define FOSC 7372800
#define MYUBRR 47
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "PWM.h"
#include "UART.h"
#include <stdbool.h>

bool newUartData = false;
unsigned char recievedData = 0;
int main(void){
	DDRD = 0xFF;
	PORTD = 0xFF;
	sei();
	UART_Init(MYUBRR);
	DDRC = 0xFF;
	PWMInit();
	int state = 0;
	int posX = 80;
	int posY = 80;
	int dataRecieved = 0;

	PORTC = 0x4;
	_delay_ms(1000);
    while(1)
    {	
		if(newUartData){
			switch (state){
				case 0:
					if(recievedData == 0xFF){
						 state = 1;
						newUartData = false;
						dataRecieved = 0;
					}
					else state = 0;
					break;
				
				case 1:
					if(recievedData == 0xFF) state = 0;
						else{
							posX = recievedData;
							newUartData = false;
							state = 2;
						}
					break;
				
				case 2:
					if(recievedData == 0xFF) state = 0;
					else{
						posY = recievedData;
						newUartData = false;
						state = 0;
					}
					break;
				
				default:
					state = 0;
					break;
			}			
		}
		
		if(posX > 160) PWMSetX(2200);
			else PWMSetX((700+(9.375*posX)));
			
		if(posY > 160) PWMSetY(2200);
			else PWMSetY((700+(9.375*posY)));	
		
		if (dataRecieved > 1000){
			PORTC = 0x8;
		}
		else{ 
			PORTC = 0x4;
			_delay_ms(4);
			dataRecieved++;
		}
	}
}

ISR(USART_RX_vect){
	
	newUartData = true;
	recievedData = UART_Receive();
	
	
}
