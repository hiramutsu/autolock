#ifndef MOTOR_H
#define MOTOR_H



/*
@desc Drives the h-bridge. If the h-bridge is in standby,
this action may not have any effect.
@input
	int dir -- 0 clockwise, otherwise counterclockwise
@output
	int -- 1 success, 0 failure
	Failure will typically occur if the h-bridge is in
	standby.
*/
int drive(int dir);

/*
@desc Stops the h-bridge.
*/
void stop();

/*
@desc Puts the h-bridge in standby mode.
@input
	int stby -- 0 no standby, otherwise standby
*/
void standby(int stby);

#endif