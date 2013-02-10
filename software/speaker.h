#ifndef SPEAKER_H
#define SPEAKER_H

#define STANDARD_A 2272  //period [us] 10^6/440


/*
@desc Plays a note on the speaker for the specified duration.
@input
	int duration -- in [s]. If duration is less than 0, nothing
	will play.
*/
void speaker_play_note(int duration);




#endif

