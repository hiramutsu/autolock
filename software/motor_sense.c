#include "motor_sense.h"
#include "AD.h"
#include "Arduino.h"

void motor_sense_enable(int en)
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

int motor_sense_is_stalled()
{
	byte en = digitalRead(MOTOR_SENSE_ENABLE);
	if(!en)
	{
		return -1;
	}
	else
	{
		analogReference(INTERNAL); //1.1V. Setup time?
		int reading = analogRead(MOTOR_SENSE_IN);
		if(reading > MOTOR_THRESHOLD)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	
	
}