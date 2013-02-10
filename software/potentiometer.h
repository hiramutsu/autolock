#ifndef POT_H
#define POT_H
#define POT_LOCKED 750
#define POT_UNLOCKED 250
/*
@desc Reads the potentiometer which measures deadbolt position
and returns its status: locked, unlocked, intermediate. Thus
function takes care of enabling and disabling the potentiometer.
@output
	int -- 0 locked, 1 unlocked, 2 intermediate
*/
char poteniometer_read();

#endif

