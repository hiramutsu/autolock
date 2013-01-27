#ifndef DOORLOCK_H
#define DOORLOCK_H

#define F_CPU 8000000L
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



#endif