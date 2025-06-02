#include "avr.h"

void avr_init(void)
{
	WDTCR = 15;
}

void mwait(unsigned short msec) 
{
	volatile int i; // so not removed by compiler
	
	// 1MHz * 500ms = 500,000 clock cycles we need to waste
	// gonna guess 3 instructions per loop; one comparison, one increment, one goto/jump
	// 500000 / 3 = 166000

	for (i = 0; i < msec; i++);
}

void avr_wait(unsigned short msec)
{
	TCCR0 = 3;  // make counting slower, too fast by default

	while (msec--) {

		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);   // set seed so one loop takes exactly 1ms

		SET_BIT(TIFR, TOV0);  // clear overflow buffer

		while (!GET_BIT(TIFR, TOV0));  // wait for overflow buffer to be 1 (meaning 1ms has passed)
	}

	TCCR0 = 0;
}

void avr_wait_us(unsigned long microsec)
{
	TCCR0 = 2;
	while (microsec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 8) * 0.00001);
		
		SET_BIT(TIFR, TOV0); // clear overflow buffer
		
		WDR();
		
		while (!GET_BIT(TIFR, TOV0)); // wait for overflow buffer to be 1 (meaning 1 mircoSec has passed)
	}
	TCCR0 = 0;
}