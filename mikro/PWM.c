/*
 * PWM.c
 *
 * Created: 08.03.2014 13:16:58
 *  Author: Erik
 */ 

#define F_CPU 7372800
#define FOSC 7372800
#include <avr/io.h>
#include <avr/interrupt.h>

void PWMInit(){
	sei();
	ICR1 = 20000;
	TCCR1A |= ( 1 << COM1B1 ) | ( 1 << COM1A1 ); // set OCnA/OCnB/OCnC on compare match (set output to high level)
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); // clk/8
	TIMSK0 |= (1 << TOIE1); //enable timer interrupt
	DDRB |= (1 << PB1);
	DDRB |= (1 << PB2);
}
void PWMSetX(int x){
			OCR1A = x;		
}

void PWMSetY(int y){
	OCR1B = y;
}
