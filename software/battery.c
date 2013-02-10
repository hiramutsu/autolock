#include "battery.h"
#include "AD.h"
#include "Arduino.h"

int battery_is_low()
{
	//Even though BATTERY_ENABLE is an output,
	//digitalRead will still return the value on the pin.
	byte en = digitalRead(BATTERY_ENABLE);
	
	if(!en)
	{
		return -1;
	}
	
	analogReference(INTERNAL); //1.1V
	int reading = analogRead(BATTERY_IN);
	
	if(reading < THRESHOLD)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void battery_enable(int en)
{
	if(en)
	{
		digitalWrite(BATTERY_ENABLE, HIGH);
	}
	else
	{
		digitalWrite(BATTERY_ENABLE, LOW);
	}
}