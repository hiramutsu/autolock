#include "motor_sense.h"
#include "AD.h"
#include "Arduino.h"

void enable(int en)
{
	if(en)
	{
		digitalWrite(MOTOR_SENSE_ENABLE, HIGH);
	}
	else
	{
		digitalWrite(MOTOR_SENSE_ENABLE, LOW);	
	}
}

int is_stalled()
{
	byte en = digitalRead(MOTOR_SENSE_ENABLE);
	if(!en)
	{
		return -1;
	}
	
	analogReference(INTERNAL); //1.1V
	int reading = analogRead(MOTOR_SENSE_IN);
	if(reading > THRESHOLD)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}