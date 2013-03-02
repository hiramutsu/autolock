#include "AD.h"
#include "speaker.h"
#include "Arduino.h"

void play_note(long notePeriodInMicrosec, long timeToPlayNoteInMicrosec);
void switchSpeakerState();


void speaker_play_note(int duration)
{
	//The Arduino assumes the chip runs at 16MHz (e.g. delayMicroseconds),
	//but the chip is actually running at 8MHz. Hence make the period 2x smaller.
	play_note(STANDARD_A/2, duration*pow(10,6));
}



void play_note(long notePeriodInMicrosec, long timeToPlayNoteInMicrosec)
{
	
	if(timeToPlayNoteInMicrosec>0){
		if(notePeriodInMicrosec <= 0){
			if(timeToPlayNoteInMicrosec>16383)   delay(timeToPlayNoteInMicrosec/1000);
			else                                 delayMicroseconds(timeToPlayNoteInMicrosec);
		}
		else{
			long noteHalfPeriod=notePeriodInMicrosec/2;
			
			while(timeToPlayNoteInMicrosec>noteHalfPeriod)
			{
				switchSpeakerState();
				delayMicroseconds(noteHalfPeriod);
				
				timeToPlayNoteInMicrosec-=noteHalfPeriod;
			}
			
			switchSpeakerState();
			delayMicroseconds(timeToPlayNoteInMicrosec);
			delayMicroseconds(16383);
		}
	}
	
	digitalWrite(SPKR, LOW);
}

void switchSpeakerState()
{
	if(bitRead(PORTD,SPKR)) digitalWrite(SPKR, LOW);
	else                          digitalWrite(SPKR, HIGH);
}