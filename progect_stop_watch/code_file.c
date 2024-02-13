/*
 * code_file.c
 *
 *  Created on: Feb 5, 2024
 *      Author: youss
 */

#include <avr/interrupt.h> // Include library interrupt
#include <avr/io.h>        // Include library I/O register
#include <util/delay.h>    // Include library delay


// Macro for individual segment bits
#define SET_BIT_0 0x01
#define SET_BIT_1 0x02
#define SET_BIT_2 0x04
#define SET_BIT_3 0x08
#define SET_BIT_4 0x10
#define SET_BIT_5 0x20
#define SET_BIT_6 0x40
#define SET_BIT_7 0x80

// Macro for half of port register
#define SET_FIRST_HALF_REG 0x0F
#define SET_LAST_HALF_REG 0xF0

// Macro for half first  of port register
#define SET_FIRST_LAST_HALF_REG 0xC0
#define SET_FIRST_TFIRST_HALF_REG 0x0C

// Macro for clear all port register
#define CLEAR_REG 0x00

// Macro for set all port register
#define SET_REG 0xFF

// Macro for set first three bits of port register
#define SET_BIT0_BIT1_BIT2 0x07

#define TIMER_TOP 1000000/1024 // Macro for timer compare value (1 sec at 1MHz clock)
#define START 0

///////////////
unsigned char pre_load = 0;
unsigned char sec_first_digit = START;
unsigned char sec_second_digit = START;
unsigned char minute_first_digit = START;
unsigned char minute_second_digit = START;
unsigned char hour_first_digit = START;
unsigned char hour_second_digit = START;
////////////////////

/*Typo: TIMER0_COMP_vect should be TIMER1_COMPA_vect*/

ISR(TIMER1_COMPA_vect) {
    sec_first_digit++;
    PORTD^=(1<<6);
    if(sec_first_digit == 10)
    {
        sec_first_digit = START;
        sec_second_digit++;
    }
    if(sec_second_digit == 6 && sec_first_digit == 0) // Logic issue: This condition should be an "else if" condition
    {
        sec_second_digit = START;
        minute_first_digit++;
    }
    if(minute_first_digit == 10) // Logic issue: This condition should be an "else if" condition
    {
        minute_second_digit++;
        minute_first_digit = START;
    }
    if(sec_second_digit == 6 && sec_first_digit == 0) // Logic issue: This condition should be an "else if" condition
    {
        hour_first_digit++;
        minute_second_digit = START;
    }
    if(hour_first_digit == 10) // Logic issue: This condition should be an "else if" condition
    {
        hour_first_digit = START;
        hour_second_digit++;
    }
    TIFR |= (1 << OCF1A); // flag
}

void timer1_init(){

// Set timer 1 for CTC mode
TCCR1A = 0;
TCCR1B = (1<<WGM12);

// Prescale of 1024 for 1MHz timer
TCCR1B |= (1<<CS12) | (1<<CS10);
// Enabole foc1a
TCCR1A|=(1<<FOC1A);

// Compare match register value for 1 second period
OCR1A = TIMER_TOP;

// Enable timer interrupt
TIMSK |= (1<<OCIE1A);


}

ISR(INT0_vect) {
	sec_first_digit = START;
	sec_second_digit = START;
	minute_first_digit = START;
	minute_second_digit = START;
	hour_first_digit = START;
	hour_second_digit = START;
	GIFR |= (1 << INTF0);
}
/////
ISR(INT1_vect) {
	TCCR1B &= 0xf8; // No Clock for paused
	GIFR |= (1 << INTF1);
}
///
ISR(INT2_vect) {
	TCCR1B |= 0x05; // Enable Clock for Resumed
	GIFR |= (1 << INTF2);
}
///////////////
void INT0_reset() {
	DDRD &= (~(1 << PD2)); // Enable internal pull-up for the pin connected to INT0
	PORTD |= (1 << PD2); // Set INT2 to trigger on the falling edge
	MCUCR &= (~(1 << ISC01)); // Clear bit to ensure falling edge
	GICR |= (1 << INT0);
}
void INT1_stop() { /// :)
	DDRD &= (~(1 << PD3));
	MCUCR |= (1 << ISC11) | (1 << ISC10);
	GICR |= (1 << INT1);
}
void INT2_active() {
	DDRB &= (~(1 << PB2)); // Enable internal pull-up for the pin connected to INT2
	PORTB |= (1 << PB2); // trigger internal pull up
	MCUCSR &= (~(1 << ISC2)); // Clear bit to ensure falling edge
	GICR |= (1 << INT2); // Enable external interrupt 2
}
//////////////////
int main()
{
	DDRD|=0b1000000;
    // Set up seven-segment display control signals
    DDRA |= SET_REG; // Output to a 7-segment display
    PORTA |= CLEAR_REG; // Turn off the 7-segment display

    DDRC |= SET_FIRST_HALF_REG; // Set as output for power
    PORTC &= SET_LAST_HALF_REG; // Initialize power control

    sei(); // Enable global interrupts

	INT0_reset();
	INT1_stop();
	INT2_active();
	timer1_init();

	while(1) {


        /*
           Update the 7-segment display based on the time values
           Display the values of sec_first_digit, sec_second_digit, minute_first_digit, minute_second_digit, hour_first_digit, and hour_second_digit
           Use appropriate delays for multiplexing the display
           */

		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_0;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (sec_first_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_1;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (sec_second_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_2;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (minute_first_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_3;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (minute_second_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_4;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (hour_first_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
		PORTA = (PORTA & SET_FIRST_LAST_HALF_REG) | SET_BIT_5;
		PORTC = (PORTC & SET_LAST_HALF_REG) | (hour_second_digit & SET_FIRST_HALF_REG);
		_delay_ms(3);
	}
}
