#include <Arduino.h>
#include <avr/io.h>
#include <hall.h>
#include <avr/cpufunc.h> 



//Lookup macro concatenation to get the pins defined properly in AD.h
void hall_init(){
	PORTB |=(1<<PB6); //controls pullup R if DDRx specifies an input pin. 1 for enabled pullup.
	DDRB |= (1<<DDB7);//direction 1 is output, 0 is input
	DDRB &= ~(1<<DDB6); // set =0 to makePB6 be an input that is pulled up
	_NOP();
	//__no_operation(); // need it for synchronization in hardware. // not sureif need one above setting DDB6 or not.
}

int hall_isClose()
{
	char isEnable = (PINB&(1<<PINB7))>>PINB7;
	if(!isEnable)
	{
		return -1;
	}
	int reading = (PINB&(1<<PINB6))>>PINB6;
	if(!reading) // assuming reading 0 means close --> pulled down.
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void hall_control(char en)
{
	if(en)
	{
		PORTB |=(1<<PB7);
	}
	else
	{
		PORTB &= ~(1<<PB7);
	}
	_NOP();
	//__no_operation();// synchronize
}