#ifndef MOTOR_SENSE_H
#define MOTOR_SENSE_H

//THRESHOLD := the level at which the A/D should detect a motor stall.
//For the case of resistor values RS = 0.28, R1 = 38K, R2 = 10K, and a stall
//current of 180 [mA], on a 1.1V scale,
#define MOTOR_THRESHOLD 80

/*
@desc Enables the motor current sense. Note that the
op amp will consume power while the sensor
is enabled.
@input
	int en -- 0 disable, otherwise enable 
*/
void motor_sense_enable(int en);

/*
@desc Checks whether the motor is stalled, based
upon how much current is flowing through the motor.
If the current exceeds the specified stall current,
a stall is reported. An error occurs if the sensor
is disabled.
@output
	int -- 1 stalled, 0 not stalled, -1 error
*/
int motor_sense_is_stalled();

#endif