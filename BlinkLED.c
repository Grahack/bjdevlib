// F_CPU tells util/delay.h our clock frequency
//#define F_CPU 8000000UL	// Orangutan frequency (8MHz)
#define F_CPU 20000000UL	// Baby Orangutan frequency (20MHz)
#include <avr/io.h>
#include <util/delay.h>

void delayms( uint16_t millis ) {
	while ( millis ) {
		_delay_ms( 1 );
		millis--;
	}
}

int main( void ) {
	DDRD |= 1 << PD1;			// set LED pin PD1 to output
	while ( 1 ) {
		PORTD &= ~( 1 << PD1 );	// LED off
		delayms( 900 );			// delay 900 ms
		PORTD |= 1 << PD1; 		// LED on
		delayms( 100 );			// delay 100 ms
	}
	return 0;
}
