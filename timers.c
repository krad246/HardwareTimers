#include <msp430.h>

char pwmOn = 1;
char isRising = 1;

#pragma vector = PORT1_VECTOR

__interrupt void onButtonPush(void) {
	pwmOn ^= 1;
	P1IFG &= ~BIT3;
}

#pragma vector = TIMER_A0_VECTOR

__interrupt void mainClock(void) {
	if (pwmOn) {
		if (TA0CCR1 >= TA0CCR0) {
			isRising = 0;
		} else if (TA0CCR1 <= 0) {
			isRising = 1;
		}

		if (isRising) {
			TA0CCR1 += 0x1;
		} else {
			TA0CCR1 -= 0x1;
		}
	}
}

#pragma vector = TIMER_A1_VECTOR 

__interrupt void changeDutyCycle(void) {
	switch (TAIV) {
	case TA0IV_TACCR1:
		P1OUT &= ~(BIT0 | BIT6);
		break;
	case TA0IV_TAIFG:
		if (TA0CCR1 > 0) {
			P1OUT |= (BIT0 | BIT6);
		}
		TA0CTL &= ~BIT0;
	}
}

int main(void) {
	// shut off watchdog timer
	WDTCTL = WDTPW | WDTHOLD;
	
	// set up LEDS
	P1OUT &= ~(BIT0 | BIT6);
	P1DIR |= (BIT0 | BIT6);
	
	// set up button and interrupts
	P1DIR &= ~BIT3;
	P1REN |= BIT3;
	P1OUT |= BIT3;
	P1IE |= BIT3;

	// change the base clock for the timer
	// it breaks without this
	BCSCTL2 = DIVS_3;

	// clear timer
	TA0CTL |= (MC_0 | TACLR);
	
	// set the timer up
	TA0CTL |= (TASSEL_2 | ID_3 | TAIE);

	// set up timer count values
	TA0CCR1 = 0x1;
	TA0CCR0 = 0x7C;

	// enable timer interrupts
	TA0CCTL0 |= CCIE;
	TA0CCTL1 |= CCIE;

	// start the timer
	TA0CTL |= MC_1;

	// GIE bit enable
	_enable_interrupts();

	// infinite loop
	for (;;);
	
	return 0;
}