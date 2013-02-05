#ifndef DEADBOLT_H
#define DEADBOLT_H

//A/D readings below the LOCKEDTH will
//mean the door is unlocked
//The threshold should be [0,1023], since the ATmega328P
//A/D is 10 bits.
//The bolt extends roughly 2.8 cm when "fully" locked. 
//The gap between the door and the frame is around 0.6 cm. 
//Therefore, threshold should be floor((max_adc-min_adc)*0.6/2.8).
//Assume the whole range of adc for now,
//LOCKEDTH = 1024*0.6/2.8 = 219
#define LOCKEDTH 219



//This is for using arduino library to read.
#define DEADBOLT_ENABLE A4
#define DEADBOLT_POSITION A3

/*
@desc Checks whether the door is locked.The motor should not
be able to drive if the reading of potentiometer is still above the threshold. 

@output
	int -- 1 locked, 0 unlocked, -1 error
*/
int deadbolt_isLocked();

/*
@desc Enables the deadbolt position sense. 
@input
	int en -- 0 disable, otherwise enable 
*/
void deadbolt_control(int en);

#endif