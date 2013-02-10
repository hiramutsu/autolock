#ifndef AD_H
#define AD_H

//Default factory setting: @3V, 8MHz + CKDIV8 --> 1MHz
//Disable CKDIV8 fuse
#define F_CPU 8000000UL
#define ARDUINO 101

//##Pin Definitions##
//See \hardware\arduino\variants\standard\pins_arduino.h 
//For the Arduino, A0 is defined as 14, A7 as 21,
//for digital use. For analog use, define the
//pin as 0 for A0. 

//Arduino_Label/ATmega328P_Label
#define BATTERY_IN 1 //A1/PC1
#define BATTERY_ENABLE 16 //A2/PC2
#define MOTOR_SENSE_IN 7 //A7/ADC7
#define MOTOR_SENSE_ENABLE 14 //A0/PC0
#define MOTOR1 10 //D10/PB2
#define MOTOR2 9 //D9/PB1
#define MOTOR_PWM 8//D8/PB0
#define MOTOR_STBY 7//D7/PD7
#define LED 5 //D5/PD5
#define SPKR 6 //D6/PD6
#define POT_ADC 3//A3/PC3
#define POT_ENABLE 18 //A4/PC4
#define XBEE_EXTRA 4 //D4/PD4

//For non-Arduino pins on the QFP package for the ATMEGA328P,
//specify the registers directly in the file (i.e. hall.*), for now.
//See macro concatenation


#endif