#ifndef BATTERY_H
#define BATTERY_H

//A/D readings below the THRESHOLD will
//be reported as a low battery.
//The threshold should be [0,1023], since the ATmega328P
//A/D is 10 bits.
//1024*(4*10/66)/1.1 = 564
//For testing, 2.66 [V] <--> 375; 3.6 V <--> 507
#define THRESHOLD 507


/*
@desc Checks whether the battery is low.The circuit
will stop functioning when the battery drops below 3.3
volts, since we are using a 3.3 V regulator. 6 AA batteries
in series will initially produce 6 [V] (1.5 V each). Note
that the Arduino init() function must be called prior
to using the A/D converter.
@output
	int -- 1 low, 0 high, -1 error
*/
int battery_is_low();

/*
@desc Enables the battery voltage sense. Note that the
voltage divider will consume power while the sensor
is enabled.
@input
	int en -- 0 disable, otherwise enable 
*/
void battery_enable(int en);

#endif