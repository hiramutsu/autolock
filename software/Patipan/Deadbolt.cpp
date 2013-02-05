/*
 * GccApplication1.cpp
 *
 * Created: 1/26/2013 4:30:48 PM
 *  Author: Boonggee
 */ 

#include <Arduino.h>
#include <avr/io.h>
//#include <util/delay.h>
//#include <pololu/orangutan.h>

#include <Deadbolt.h>


int main(void)
{
	init();
    while(1)
    {
		
		
    }
}

int deadbolt_isLocked()
{
	pinMode(DEADBOLT_ENABLE, OUTPUT);
	char isEnable = digitalRead(DEADBOLT_ENABLE); // not sure if can call digiread from output pin
	if(!isEnable)
	{
		return -1;
	}	
	analogReference(DEFAULT); //AVCC = 3.3 V
	pinMode(DEADBOLT_POSITION, INPUT);
	int reading = analogRead(DEADBOLT_POSITION);
	if(reading > LOCKEDTH)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void deadbolt_control(char en)
{
	pinMode(DEADBOLT_ENABLE, OUTPUT);
	if(en)
	{
		digitalWrite(DEADBOLT_ENABLE, HIGH);
	}
	else
	{
		digitalWrite(DEADBOLT_ENABLE, LOW);
	}
}
