#include "potentiometer.h"
#include "AD.h"
#include "Arduino.h"

char poteniometer_read()
{
	digitalWrite(POT_ENABLE, HIGH);
	delay(10);
		
	analogReference(DEFAULT); //3.3V
	char status;
	int reading = analogRead(POT_ADC);
	
	if(reading > POT_UNLOCKED)
		status = 1;
	else if(reading < POT_LOCKED)
		status = 0 ;
	else
		status = 2;	
	
	digitalWrite(POT_ENABLE, LOW);
	return status;	
}