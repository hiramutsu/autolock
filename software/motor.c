#include "motor.h"
#include "AD.h"
#include "Arduino.h"

/*
@desc Drives the bridge inputs of the TB6552FNG dual h-bridge
*/
void motor(int in1, int in2, int standby, int pwm);

void motor(int in1, int in2, int standby, int pwm)
{
	digitalWrite(MOTOR1, in1);
	digitalWrite(MOTOR2, in2);
	digitalWrite(MOTOR_STBY, standby);
	digitalWrite(MOTOR_PWM, pwm);
}

int drive(int dir)
{
	if(!digitalRead(MOTOR_STBY))
	{
		return 0;
	}
	
	//The h-bridge is in standby is the pin is held 0.
	//If pwm is low, the h-bridge short brakes.
	if(dir)
	{
		motor(1,0,1,1);
	}
	else
	{
		motor(0,1,1,1);
	}
	
	return 1;
}

void stop()
{
	//Maintain the current state of MOTOR_STBY.
	motor(0,0,digitalRead(MOTOR_STBY),1);
}

void standby(int stby)
{
	if(stby)
	{
		digitalWrite(MOTOR_STBY, LOW);
	}
	else
	{
		digitalWrite(MOTOR_STBY, HIGH);
	}
}