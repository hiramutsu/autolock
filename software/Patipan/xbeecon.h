#ifndef XBEECON_H
#define XBEECON_H


//This is for using arduino library to read.
#define XBEE_SLP 3  //PD3
#define XBEE_CTS  2 //PD2

/*
@desc Read whether the xbee is properly waken up and ready for data transmission. 

@output
	int -- 1 clear(pin low), 0 not ready (pin high), -1 error
*/
int xbee_cts();

/*
@desc put xbee into sleep.
@input
	int slp -- 0 = wake up, sleep otherwise
*/
void xbee_sleep(char slp);


#endif