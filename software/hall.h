#ifndef HALLEFFECT_H
#define HALLEFFECT_H


//This is for using arduino library to read.
//#define HALL_ENABLE  //PB7
//#define HALL_SIGNAL  //PB6

/*
@desc Checks whether the door is closed.The motor should not
be able to drive if the reading of potentiometer is still above the threshold. 

@output
	int -- 1 closed, 0 ~closed, -1 error
*/
int hall_isClose();

/*
@desc Enables the Hall Effect sensor.
@input
	int en -- 0 disable, otherwise enable 
*/
void hall_control(char en);


/*
@desc Initialize the registers setting for hall effect sensor

@output
	-
*/
void hall_init();

#endif