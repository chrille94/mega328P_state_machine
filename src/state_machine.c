/*
 * state_machine.c
 *
 * Created: 23-02-2018 11:05:42
 * Author : criis
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define RED_ON		PORTC|=(1<<PORTC4);		//Macros for turning on and off LEDs
#define RED_OFF		PORTC&=~(1<<PORTC4);
#define GREEN_ON	PORTC|=(1<<PORTC5);
#define GREEN_OFF	PORTC&=~(1<<PORTC5);
#define YEL_ON		PORTC|=(1<<PORTC3);
#define YEL_OFF		PORTC&=~(1<<PORTC3);
#define BLUE_ON		PORTC|=(1<<PORTC2);
#define BLUE_OFF	PORTC&=~(1<<PORTC2);

volatile unsigned char btn1 = 0, btn2 = 0, tik = 0;		//Interrupt flags initialized

void initPins() {
	DDRC|=(1<<DDC2)|(1<<DDC3)|(1<<DDC4)|(1<<DDC5);		//LED pins -> output
	PORTD|=(1<<PORTD2)|(1<<PORTD3);						//Pullup on INT0 an INT1
	EICRA|=(1<<ISC01)|(1<<ISC11);						//Falling edge on INT0 and INT1
	EIMSK|=(1<<INT0)|(1<<INT1);							//enable external interrupts
}

void init_spi() {
	DDRB |= (1<<DDB2)|(1<<DDB3)|(1<<DDB5);	//SS, MOSI and SCK -> outputs
	SPCR |= (1<<SPE)|(1<<MSTR);			//Enable SPI as master
}

void initTimer()	//Sets up timer 1 to create an interrupt every second
{
	//TCCR1A|=(1<<COM1A0);//|(1<<WGM11);
	TCCR1B|=(1<<WGM12)|(1<<CS12)|(1<<CS10);  //CTC on timer 1, with 1024 prescaling
	OCR1A=15624;	//16MHz
	//OCR1A=7812;		//8MHz
	//TIMSK1|=(1<<OCIE1A); //Enable timer interrupt
}

ISR(TIMER1_COMPA_vect) {		//ISR to set flag
	tik = 1;
}

ISR(INT0_vect) {
	btn1 = 1;
}

ISR(INT1_vect) {
	btn2 = 1;
}

void updateDisplay(unsigned char data) {	//output to shift register via SPI
	PORTB&=~(1<<PORTB2);			//Latch low
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));	//wait for transmit completion
	PORTB|=(1<<PORTB2);			//Latch high
}

int main(void)
{
	const char sevenSeg[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
		0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};	//"decoded" 7-segment values
	unsigned char count = 5;
	init_spi();
	initTimer();
	initPins();
    enum states {Red, Green, Blue, Yellow};
	enum states state, nextstate;		//create two instances of the 'states' enum type - one for current state and one for next
	state = Red;
	nextstate = Red;
	updateDisplay(0);	//make sure the display is off by sending all zeros to the shift register
	sei();
    while (1) 
    {
		switch(state) {
			case Red:
				YEL_OFF
				GREEN_OFF
				BLUE_OFF
				RED_ON
				if(btn2) {nextstate = Yellow;	btn2 = 0;}
				if(btn1) {nextstate = Green;	btn1 = 0;}
			break;
			case Green:
				YEL_OFF
				RED_OFF
				BLUE_OFF
				GREEN_ON
				if(btn2) {nextstate = Yellow;	btn2 = 0;}
				if(btn1) {nextstate = Blue;		btn1 = 0;}
			break;
			case Blue:
				YEL_OFF
				GREEN_OFF
				RED_OFF
				BLUE_ON
				if(btn2) {nextstate = Yellow;	btn2 = 0;}
				if(btn1) {nextstate = Red;		btn1 = 0;}
			break;
			case Yellow:
				if(!(TIMSK1 & (1<<OCIE1A))) {	//enable timer interrupt
					TIMSK1|=(1<<OCIE1A);
				}
				RED_OFF
				GREEN_OFF
				BLUE_OFF
				YEL_ON
				if(tik && count > 0) {				//check timer interrupt flag and decrement count if it's larger than 0
					updateDisplay(sevenSeg[count]);
					--count;
					tik = 0;
				}else if(tik && count == 0) {
					updateDisplay(0);
					tik = 0;
					nextstate = Red;
					count = 5;
				}
				if(btn2) {
					TIMSK1&=~(1<<OCIE1A);	//Disable timer interrupt
					nextstate = Red;	
					btn2 = 0;
					count = 5;
					updateDisplay(0);
				}
			break;
			default: break;
		}
		state = nextstate;	//Save the nextstate in state, so that the the new state will be executed next time the loop runs
    }
}

