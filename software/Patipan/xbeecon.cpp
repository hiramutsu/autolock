/*
 * GccApplication1.cpp
 *
 * Created: 1/26/2013 4:30:48 PM
 *  Author: Boonggee
 */ 

#include <Arduino.h>
#include <avr/io.h>
#include <xbeecon.h>
#include <stdio.h>

int xbee_cts()
{
	pinMode(XBEE_CTS, INPUT);
	char is_cts = digitalRead(XBEE_CTS);
	if(!is_cts)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void xbee_sleep(char slp)
{
	pinMode(XBEE_SLP, OUTPUT);
	if(slp)
	{
		digitalWrite(XBEE_SLP, HIGH);
	}
	else
	{
		digitalWrite(XBEE_SLP, LOW);
	}
}

int xbee_isMsg()
{
	pinMode(XBEE_MSG, INPUT);
	if(digitalRead(XBEE_MSG))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
