/*
@desc Test the A/D converter
@author Sriram Jayakumar
@date 12/10/2012
*/

#include "AD.h"
#include "motor.h"
#include "motor_sense.h"
#include "speaker.h"
#include "Arduino.h"
#include "hall.h"
#include "motor_sense.h"
#include "battery.h"
#include "potentiometer.h"
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <avr/pgmspace.h>

void setup();
void loop();
void setup_adc();
void test_motor();

void setup()
{
	//Arduino initialization. E.g. AD converter enables.
	init();
	
	pinMode(BATTERY_ENABLE, OUTPUT);
	pinMode(MOTOR_SENSE_ENABLE, OUTPUT);
	pinMode(MOTOR_PWM, OUTPUT);
	pinMode(MOTOR_STBY, OUTPUT);
	pinMode(MOTOR1, OUTPUT);
	pinMode(MOTOR2, OUTPUT);
	pinMode(SPKR, OUTPUT);
	pinMode(POT_ENABLE, OUTPUT);
	//pinMode(XBEE_EXTRA, INPUT);
	
	digitalWrite(POT_ENABLE, LOW);
	motor_standby(0);
	hall_init();
	hall_control(0);
	motor_sense_enable(1);
	battery_enable(0);
	
}

void loop()
{
	test_lock_unlock();
	
}

void test_motor_sense()
{
	if(motor_sense_is_stalled())
	{
		speaker_play_note(2);
	}
	
	delay(2000);
	
}


void test_motor()
{
	motor_drive(0);
	delay(1000);
	motor_drive(1);
	delay(1000);
}

void test_lock_unlock(int lock)
{
	//int lock = digitalRead(XBEE_EXTRA); //0 = lock. 1 = unlock.
	int status = poteniometer_read();
	
	if(status!=lock)
	{
		motor_sense_enable(1);
		motor_standby(0);
		motor_drive(lock);
		
		while(1)
		{
			if(motor_sense_is_stalled() || poteniometer_read()==lock)
				break;
			else
				delay(10);
		}			
		
		motor_stop();
		motor_sense_enable(0);
		motor_standby(1);
	}
	
	//Error: intermediate deadbolt position
	if(poteniometer_read()==2)
	{
		speaker_play_note(1);
	}		
	
	delay(5000);
}
//use polling because interrupt is overflow.
static char csum;
static void uart_putc (unsigned char v) { while (!(UCSR0A&(1<<UDRE0))); UDR0 = v; }// this checks if the the UART line is ready. if not, spins wait until it's ready.
static void uart_putcc (unsigned char v) { while (!(UCSR0A&(1<<UDRE0))); UDR0 = v; csum += v; }// this one also calculate checksum ASSUMING IT IS CLEAR TO 0 before start.
static unsigned char uart_kbhit (void) { return(UCSR0A&(1<<RXC0)); }
static unsigned char uart_getch (void) { return(UDR0); }
static void uart_init (unsigned int baudval)
{
   UBRR0H = (baudval>>8); UBRR0L = (baudval&255);
   UCSR0A = (1<<U2X0); //double speed mode
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);
   UCSR0C = (0<<USBS0)|(3<<UCSZ00); //N81
}

// for AP1  only
// leng is the length of AT Command 
static void uart_cmd (unsigned char *buf, unsigned char leng)
{
    unsigned char i, csum;
	uart_putc(0x7e); // The header for AP1 packet
	uart_putc(0); uart_putc(leng+2); // length of AP1 packet (total number of bytes inside packet not counting the header & checksum   MSB first)
	//add 2 because of the 2 fixed frame (example for the AT command is wrong in AP1 because the length should be 4 not 5)
	
	csum = 0;
	uart_putc(0x08); csum += 0x08;
	uart_putc(0x01); csum += 0x01;
	for(i=0;i<leng;i++)
		{ uart_putc(buf[i]); csum += buf[i];	}
		   	
	uart_putc(~csum);

}
void uart_data(char *data, uint16_t leng, uint32_t dstip, uint16_t dstport, uint16_t srcport){// uint16_t is 16 bit integer=short, _t stands for type.
	
	//unsigned char csum=0;
	csum =0; // doesn't define local; therefore, use global one
	uint16_t i =0;
	uart_putc(0x7e);
	uart_putc((leng+1+1+4+2+2+1+1) >> 8); // divide by 256
	uart_putc((leng+1+1+4+2+2+1+1) & 255); // mod 256
	uart_putcc(0x20);
	uart_putcc(0x01);
	uart_putcc(dstip>>24);
	uart_putcc((dstip>>16)&255);
	uart_putcc((dstip>>8)&255);
	uart_putcc(dstip&255);
	
	uart_putcc(dstport>>8);
	uart_putcc(dstport&255);
	
	uart_putcc(srcport>>8);
	uart_putcc(srcport&255);
	
	uart_putcc(0x01);// 1= TCP
	uart_putcc(0x02); // 2 means BIT 1 = 1; therefore, terminate socket after tx completes
	
	for (i = 0; i < leng; i++)
	{
		uart_putcc(data[i]);
	}
	uart_putc(~csum);
	
}

//send api command in mode AP1, buf is a string of command 
void sendcmd1 (char *buf, uint16_t leng)
{
	unsigned char csum=0;

	uart_putc(0x7e);
	//it's leng +2 because the0x08 and 0x01 needs to be computed in checksum and is hidden from callers
	uart_putc((leng+2)>>8);
	uart_putc((leng+2)&255);
	uart_putc(0x08);
	uart_putc(0x01);
	csum +=0x08;
	csum +=0x01;
	uart_putc(~csum);
	uart_putc(leng>>8);
}

//ONLY FOR AP0 return true when the programming mode is successfully entered.
int enterProgModeAP0(void) {
	//for storing the reply from xbee
	char rep[4];
	char c;
	//bad coding style
	rep[0] = ' ';
	rep[1] = ' ';
	rep[2] = ' ';
	rep[3] = '\0';
	int i=0;
	
	//send '+++' to enter AT mode
	_delay_ms(1200);
	uart_putc('+');
	uart_putc('+');
	uart_putc('+');
	//listening until get "OK\r"
	//TODO: Implement time out 
	while(1){
		while (uart_kbhit())
			{
				c = uart_getch();
				rep[i] = c;
				if(strcmp(rep,"OK\r")==0){
					return 1;
				}
				i++;
			}
	}	
}
//note: to check page use memcmp (&buf, "hello.htm". 9) ==0; // note that &buf give the beginning of memory, we then compare it with the string"hello.htm" for 9 characters
//therefore the null terminator (\0) at the end of the string is ignored

void uart_putbuf(unsigned char *buf, uint16_t leng){
	for (uint16_t i =0 ; i<leng;i++){
		uart_putc(buf[i]);
	}
	
}

void sendpage(char *buf, PGM_P page, char *lenbuf, uint32_t dstip, uint16_t dstport, uint16_t srcport){
	itoa(strlen(page),lenbuf,10);
	
	strcpy(buf,
	"HTTP/1.1 200 OK\r\n"
	"Server: Ken's g&a XBee-WIFI server\r\n"
	"Content-length: ");
	strcat(buf,lenbuf);
	strcat(buf,"\r\n"
	"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
	"\r\n");
	strcat_P(buf, page);
	
	uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
	
	
}




int main (void)
{
	//Call setup to initialize the pin connections, A/D converter, and other devices.
	//Uncomment this to have a working version of the code.
	//setup(); 
	
  char buf[1024];// This already takes half of the memory so we need to reuse the buffer for both rx and tx
  int i, rbufi = 0, rleng = 0;
  
  char lenbuf[8]; // will store length of the pages wished to sent 
 
  //pages defined here
  //-------------------------------------------------------------------------------------------
  
  static const char PROGMEM homepage[] ="<html>"
  "<title>Door Lock Homepage</title>"
  "<body>"
  "<h2 align=center>Log in to the (un)secured server</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"credential.htm\">"
  "<br>User: <input type=\"text\" name =\"user\" value = \"Username\" size = \"60\"></br>"
  "<br>Password: <input type=\"password\" name =\"passwd\" size = \"60\"></br>"
  "<br><input type = \"submit\" name = \"tosend\" value = \"Login\"></br>"
  "</form>"
  "</body>"
  "</html>"; // put it in program memory because we have 16 kB
  
  static const char PROGMEM lockpage[] ="<html>"
  "<title>Door Lock Control Page</title>"
  "<body>"
  "<h2 align=center>Lock Control</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"Control.htm\">"
  "<br><input type = \"submit\" name = \"tolock\" value = \"Lock\"></br>"
  "<br><input type = \"submit\" name = \"tounlock\" value = \"Unlock\"></br>"
  "</form>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"status.html\">"
  "<br><input type = \"submit\" name = \"tost\" value = \"Status\"></br>"
  "</form>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"index.html\">"
  "<br><input type = \"submit\" name = \"logout\" value = \"Log Out\"></br>"
  "</form>"
  "</body>"
  "</html>";
  
  static const char PROGMEM invalidpage[] ="<html>"
  "<title>Error Occurred</title>"
  "<body>"
  "<h2 align=center>Invalid Username/password</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"index.html\">"
  "<br><input type = \"submit\" name = \"back\" value = \"Back\"></br>"
  "</form>"
  "</body>"
  "</html>";


//haven't tested these with integration
  static const char PROGMEM statuspagep1[] ="<html>"
  "<title>Status</title>"
  "<body>"
  "<h2 align=center>Device Status</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"credential.html\">" //cheating to bypass the page right now. Notice the ".html" not ".htm"
  "<br>Door Position: ";
  
  static const char PROGMEM statuspagep2[] ="</br>"
  "<br>Lock Position: ";
  static const char PROGMEM statuspagep3[] ="</br>"
  "<br>Battery level: ";
  
  static const char PROGMEM statuspagep4[] ="</br>"
  "<br><input type = \"submit\" name = \"back\" value = \"Back\"></br>"
  "</form>"
  "</body>"
  "</html>";
  
  //-------------------------------------------------------------------------------------------
   #define UART_BAUD 9600
   uart_init(F_CPU/8/UART_BAUD-1);
	//sei();//global interrupt enable
  #if 0
	if(enterProgModeAP0()){
		
		//uart_putc('A');  uart_putc('T');  uart_putc('A');  uart_putc('P'); uart_putc('1'); uart_putc(',');  uart_putc('W');  uart_putc('R'); uart_putc('\r'); delay_ms(100);

	}
	while (1)
	{
	}
	#endif

	sendcmd1("IP\x01",3); //Listen on TCP
	sendcmd1("C0\x00\x80",4); 
	 while(1){
		 while (uart_kbhit())
		 {
			i = uart_getch();

			if (((rbufi <= 0) || (rbufi >= rleng+4)) && (i == 0x7e)) {
				rbufi = 0;
			}				
			
			if (rbufi < sizeof(buf)) { 
				buf[rbufi] = i;		
			} 
			rbufi++;
			if (rbufi == 3) { 
				rleng = (((int)buf[1])<<8) + (int)buf[2]; 
			
				
				if (rleng > sizeof(buf)) 
					rleng = sizeof(buf); 
			} //this is checking the length from packet.
			if (rbufi == 1+2+rleng+1)//check the whole length of packet (add rleng with 7e, 2 byte length, and 1 byte checksum)
			{
													
					//XBee packet header bytes:
					//   Tx64 Request                       0x00
					//   Rx64 Indicator                     0x80
					//
					//   AT Command                         0x08
					//   AT Command - Queue Parameter Value 0x09
					//   AT Command Response                0x88
					//
					//   Remote Command Request             0x07
					//   Remote Command Response            0x87
					//
					//   TX IPv4                            0x20
					//   RX IPv4                            0xb0
					//
					//   TX Status                          0x89
					//   Modem Status                       0x8a
					//   IO Data Sample Rx Indicator        0x8f
				if (buf[3] == 0xb0) //Xbee-WIFI packet format: 0x7e lenghi lenglo [ 0xb0 [IPV4 port port ]] chksum
				{
					
						//HTTP receive from IE8 (NOTE:all fields big endian):
						//char header = 0x7e;
						//short leng = 0x0236;
						//char ipv4rxhead = 0xb0;
						//int ip = 0xc0a80164; //192.168.1.100
						//short dstport = 80;
						//short srcport = 55492;
						//char protocol = 0x01; //0=UDP, 1=TCP
						//char status = 0x00; //reserved
						//char "GET / HTTP/1.1\r\n";
						//char "Accept: application/x-ms-application, image/jpeg, application/xaml+xml, image/gif, image/pjpeg, application/x-ms-xbap, application/x-shockwave-flash, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\n";
						//char "Accept-Language: en-US\r\n";
						//char "User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.1; WOW64; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; InfoPath.2; .NET4.0C; .NET4.0E)\r\n";
						//char "Accept-Encoding: gzip, deflate\r\n";
						//char "Host: 192.168.1.107\r\n";
						//char "Connection: Keep-Alive\r\n";
						//char "\r\n";
						//char chksum = 0xcd;
					 
						 uint32_t dstip = ((uint32_t)buf[4]<<24)+((uint32_t)buf[5]<<16)+((uint32_t)buf[6]<<8)+(uint32_t) buf[7]; //destination IP address to send back
						 uint16_t dstport= ((uint16_t) buf[10]<<8)+((uint16_t)buf[11]); 
						 uint16_t srcport = ((uint16_t) buf[8]<<8) + ((uint16_t)buf[9]);
						//if not cast as int, the shifting will give 0 because it will think it's char(1 bytes)
					
					if(!memcmp(&buf[14],"GET",3)){
						for(i=20;(i < 1+2+rleng-4) && (memcmp(&buf[i],"HTTP/1",6));i++)
						{;}	
						if((!memcmp(&buf[17]," / ",3))||(!memcmp(&buf[17]," /index.html ", 13))){
							sendpage(buf, homepage, lenbuf, dstip, dstport, srcport);
						}
					}else if (!memcmp(&buf[14],"POST",4)){
						if((!memcmp(&buf[18]," /credential.htm ", 17))||(!memcmp(&buf[18]," /credential.html ", 18))){
							uint16_t ff =0; // use to find a user field
							uint16_t gg =0; // use to find a password field
							for(ff=35;(ff < 1+2+rleng-4) &&(ff<1024)/*length of buffer*/&& (memcmp(&buf[ff],"name=\"user\"\r\n\r\n",15));ff++)
							{;}	
							for(gg=ff;(gg < 1+2+rleng-4) &&(gg<1024)/*length of buffer*/&& (memcmp(&buf[gg],"name=\"passwd\"\r\n\r\n",17));gg++)
							{;}
							
							if((!memcmp(&buf[18]," /credential.html ", 18))||(!memcmp(&buf[ff+15],"test\r\n",6)&&!memcmp(&buf[gg+17],"test\r\n",6))){
								sendpage(buf, lockpage, lenbuf, dstip, dstport, srcport);
							}else{								
								sendpage(buf, invalidpage, lenbuf, dstip, dstport, srcport);
							}								
						}else if(!memcmp(&buf[18]," /index.html ", 13)){
							sendpage(buf, homepage, lenbuf, dstip, dstport, srcport);
						}else if(!memcmp(&buf[18]," /Control.htm ", 14)){
					
							uint16_t ff =0; // use to find a user field
							uint16_t gg =0; // use to find a password field
							for(ff=35;(ff < 1+2+rleng-4) &&(ff<1024)/*length of buffer*/&& (memcmp(&buf[ff],"name=\"tolock\"\r\n\r\n",17));ff++)
						{;}
							for(gg=ff;(gg < 1+2+rleng-4) &&(gg<1024)/*length of buffer*/&& (memcmp(&buf[gg],"name=\"tounlock\"\r\n\r\n",19));gg++)
						{;}
					
							if(!memcmp(&buf[ff+17],"Lock\r\n",6)){
								test_lock_unlock(0);
							}else if(!memcmp(&buf[gg+19],"Unlock\r\n",8)){
								test_lock_unlock(1);
							}			
							sendpage(buf, lockpage, lenbuf, dstip, dstport, srcport);		
					
						}else if(!memcmp(&buf[18]," /status.html ", 14)){
							//WILL NEED TO be VERY CAREFUL WITH THE STRLEN, PROCESS IT BEFORE DO THIS.
							itoa(strlen(statuspagep1)+strlen(statuspagep2)+strlen(statuspagep3)+strlen(statuspagep4),lenbuf,10);
							strcpy(buf,
							"HTTP/1.1 200 OK\r\n"
							"Server: Ken's g&a XBee-WIFI server\r\n"
							"Content-length: ");
							strcat(buf,lenbuf);
							strcat(buf,"\r\n"
							"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
							"\r\n");
							strcat_P(buf,statuspagep1);
							//strcat door position here
							strcat_P(buf,statuspagep2);
							//strcat lock status here
							strcat_P(buf,statuspagep3);
							//strcat battery status here
							strcat_P(buf,statuspagep4);
							uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);						
							
						}
				}
							
				}
				
			}	
			
		 }
		 
		
	 } 
	
   
}
void setup_adc()
{
	//####
	//Enable the ADC converter by setting the appropriate
	//registers. Set the prescaler for the AD to divide
	//the main clk by 128. The AD clock should be between
	//50-200 kHz. Lower frequencies provide more accuracy.
	
	//Power reduction
	PRR &= ~(_BV(PRADC));
	
	//Prescaler
	ADCSRA |= _BV(ADPS0);
	ADCSRA |= _BV(ADPS1);
	ADCSRA |= _BV(ADPS2);
	
	//Enable
	ADCSRA |= _BV(ADEN);
}

//int main(void)
//{
	//setup();
	//
	//while(1)
	//{
		//loop();
	//}
//}
//
