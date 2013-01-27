///*
//@desc Test the A/D converter
//@author Sriram Jayakumar
//@date 12/10/2012
//*/
//
////###Constants###
//#define F_CPU 8000000L
//#define ARDUINO 101
//#include "Arduino.h"
//void setup();
//void loop();
//
////PortName ATMEGA_Pin_No. See \hardware\arduino\variants\standard\pins_arduino.h 
//const int LED = 0; //PD0 2
//const int LED2 = 5; //PD5 11
//
////###Variables###
////Whether a stall has been detected
//int stall;
//
//void setup()
//{
	////####
	////Enable the ADC converter by setting the appropriate
	////registers. Set the prescaler for the AD to divide
	////the main clk by 128. The AD clock should be between
	////50-200 kHz. Lower frequencies provide more accuracy.
	//
	////Power reduction
	//PRR &= ~(_BV(PRADC));
	//
	////Prescaler
	//ADCSRA |= _BV(ADPS0);
	//ADCSRA |= _BV(ADPS1);
	//ADCSRA |= _BV(ADPS2);
	//
	////Enable
	//ADCSRA |= _BV(ADEN);
	//
	////Arduino initialization
	//init();
	//
	////####
	////Pins
	//analogReference(INTERNAL);
	//pinMode(LED, OUTPUT);
	//pinMode(LED2, OUTPUT);
	//pinMode(AIN1, OUTPUT);
	//pinMode(AIN2, OUTPUT);
	//pinMode(PWM, OUTPUT);
	//pinMode(ASTANDBY, OUTPUT);
//
	//digitalWrite(LED, LOW);
	//drive();
	//
	//
//}
//
//void loop()
//{
	//if(!stall)
	//{
		////The maximum sampling rate of the ATmega328P
		////A/D is 10 [kHz], for reference. Here, sample
		////every ms.
		//delay(100);
		//int data = analogRead(MOTORSENSE);
		//digitalWrite(LED2, data%2);
		//if(data <= THRESHOLD)
		//{
			//digitalWrite(LED, LOW);
		//}
		//
		//else
		//{
			//digitalWrite(LED, HIGH);
			//stall = 1;
			//stop();
		//}
	//}
	//
//}
//

#include "AD.h"
#include "motor.h"
#include "motor_sense.h"
#include "Arduino.h"

void setup();
void loop();

void setup()
{
	//Arduino initialization. E.g. AD converter enables.
	init();
	
	pinMode(BATTERY_ENABLE, OUTPUT);
	pinMode(MOTOR_SENSE_ENABLE, OUTPUT);
	pinMode(MOTOR_PWM, OUTPUT);
	pinMode(MOTOR_STBY, OUTPUT);
	pinMode(MOTOR1, OUTPUT);
	pinMode(MOTOR2, OUTPUT);
	pinMode(SPKR, OUTPUT);
	
	standby(0);
}

void loop()
{
	drive(1);
	delay(50);
	stop();
	delay(100);
	drive(0);
	delay(150);
	stop();
	delay(200);
}

int main(void)
{
	setup();
	
	while(1)
	{
		loop();
	}
}

