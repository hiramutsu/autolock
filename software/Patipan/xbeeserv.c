/* orangutan_app2 - an application for the Pololu Baby Orangutan B
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 10/22/2012 3:24:47 PM
 *  Author: Boonggee
 *  Support code from: Ken Silverman
 */

#define F_CPU 20000000UL //Baby Orangutan core frequency is 20MHz
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <pololu/orangutan.h>
#include <stdio.h>
//#include <conio.h>
#include <math.h>
//#include <Arduino.h>


static void motor_init (void)
{
	TCCR0A = (1<<WGM00)|(0<<WGM01) | (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0); //Timer0 stuff
	TCCR0B = (0<<WGM02) | (1<<CS01); //Timer0:prescaler=8
	TIFR0  = (1<<TOV0);              //Clear TOV0 / clear pending interrupts
	DDRD  |= (1<<DDD5)|(1<<DDD6);    //Set Port M1A&M1B as outputs
	OCR0A = 0; OCR0B = 0;
}
static void motor_setvel (int i)
{
if (i >= 0) { OCR0A = 0; if (i > 255) { i = 255; } OCR0B = i; }
          else { OCR0B = 0; if (i <-255) { i =-255; } OCR0A =-i; }
}

static void led_init (void) { DDRD |= (1<<DDD1); }
static void led_set (int i) { if (!i) PORTD &= ~(1<<PORTD1); else PORTD |= (1<<PORTD1); }

static void buts_init (void) { DDRD &= ~((1<<DDD4)|(1<<DDD7)); } //PD4,PD7=inputs
static char buts_get (void) { return((((PIND>>4)&1) + ((PIND>>6)&2)) ^ 3); }


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

//for AP1  only
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
void uart_data(unsigned char *data, uint16_t leng, uint32_t dstip, uint16_t dstport, uint16_t srcport){// uint16_t is 16 bit integer=short, _t stands for type.
	
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
void sendcmd1 (unsigned char *buf, uint16_t leng)
{
	uint16_t i;
	unsigned char csum=0;

	uart_putc(0x7e);
	//it's leng +2 because the0x08 and 0x01 needs to be computed in checksum and is hidden from callers
	uart_putc((leng+2)>>8);
	uart_putc((leng+2)&255);
	uart_putc(0x08);
	uart_putc(0x01);
	csum +=0x08;
	csum +=0x01;
	/*
	for(i=0;i<leng;i++){
		 uart_putc(buf[i]);
		 csum += buf[i];
	}
	*/		 
	uart_putc(~csum);
	uart_putc(leng>>8);
}

//ONLY FOR AP0 return true when the programming mode is successfully entered.
int enterProgModeAP0(void) {
	//for storing the reply from xbee
	unsigned char rep[4];
	unsigned char c;
	//bad coding style
	rep[0] = ' ';
	rep[1] = ' ';
	rep[2] = ' ';
	rep[3] = '\0';
	int i=0;
	char ok[] = "OK\r";
	
	//send '+++' to enter AT mode
	delay_ms(1200);
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

//this is doing strcat with a string in program memory. There's a defined program called strcat_P
static void strcat_pgm (char *buf, PGM_P bufadd /*const char *bufadd*/){ //PGM_P is for program memory
	unsigned char ch;
	while (*buf){
		buf++;
	}
	do{
		ch = pgm_read_byte(bufadd);
		bufadd++;
		*buf = ch;
		buf++;
	}while (ch);
}



int main (void)
{
  // unsigned char buts = 0;
  unsigned char buf[1024];//[768];// This already takes half of the memory so we need to reuse the buffer for both rx and tx
  //buffer of size 1024 is too big. 
  int i, j, rbufi = 0, rleng = 0;
  
  char lenbuf[8]; // will store length of the pages wished to sent 
  
  //the following 4 is the part that is untested, NEED TO RESET THEM LIKE rbufi too
  /*char countdash = 0;//count consecutive dash
  char countline = 0;//count dash line
  char checkpost = 0; //1  if post request
  char checkhttp = 0; // see if reaches HTTP/1.1 or something like that
  
  int tbufi =0;//true buf index
*/
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


//haven't tested these two
  static const char PROGMEM successlockpage[] ="<html>"
  "<title>Locked</title>"
  "<body>"
  "<h2 align=center>Lock succeeded</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"credential.htm\">"
  "<br><input type = \"submit\" name = \"back\" value = \"Back\"></br>"
  "</form>"
  "</body>"
  "</html>";
  
  static const char PROGMEM successunlockpage[] ="<html>"
  "<title>Locked</title>"
  "<body>"
  "<h2 align=center>Lock succeeded</h2><br>"
  "<form method = \"post\" enctype=\"multipart/form-data\" action=\"credential.htm\">"
  "<br><input type = \"submit\" name = \"back\" value = \"Back\"></br>"
  "</form>"
  "</body>"
  "</html>";
  //-------------------------------------------------------------------------------------------
   #define UART_BAUD 9600
   uart_init(F_CPU/8/UART_BAUD-1);
	//sei();//global interrupt enable
	unsigned char c = 0;
  #if 0
	if(enterProgModeAP0()){
		
		//uart_putc('A');  uart_putc('T');  uart_putc('A');  uart_putc('P'); uart_putc('1'); uart_putc(',');  uart_putc('W');  uart_putc('R'); uart_putc('\r'); delay_ms(100);

	}
	while (1)
	{
	}
	#endif

	//bufcpy(buf, tcpen, 3);
	//memcpy(buf,tcpen,3);//strlen(tcpen)+1); //need to put \0 otherwise doesn't work don't know why. Also don't know why strlen doesn't work
	sendcmd1("IP\x01",3); //Listen on TCP
	sendcmd1("C0\x00\x80",4);
	//sprintf(buf,"C0%c%c",80>>8,80&255); sendapi1(buf,4); //Listen on port 80
	 while(1){
		//char prevread = 0; // for storing previous value
		 while (uart_kbhit())
		 {
			// uart_putc(i == 0x7e);
			 //uart_putc(i);
			// prevread = i; // store previous value
			i = uart_getch();
			//uart_putc(i);
	//			printf("%02x ",i); //continue;

			if (((rbufi <= 0) || (rbufi >= rleng+4)) && (i == 0x7e)) {
				rbufi = 0;
			//	tbufi = 0; // Patipan : I added this line to reset tbufi the same way rbufi does. Probably still need to reset those 4 fields for form post as well
			}				
			
			if (rbufi < sizeof(buf)) { 
				buf[rbufi] = i;		
				/*
				if(checkhttp&&checkpost&&(countline<2)){//if post request and already past HTTP/1.1 thing
					if (!memcmp(&i,"-",1))
					{
						
						countdash++;
						if(countdash>18){
							countline++;
							countdash = 0;
						}
					}else{
						countdash = 0;
					}
				}else{			
					//need a check if past HTTP/1.1	
					//while(rbufi==20){;}
					buf[tbufi] = i;
					//uart_putc(buf[tbufi]);
					//uart_putc(tbufi);
					tbufi++;
					if (!checkpost&&(rbufi>13)&&(!memcmp(&buf[14],"POST",4))){
						checkpost=1;
					}
					if(!checkhttp&&(rbufi>2)&&(!memcmp(&buf[rbufi-3],"HTTP", 4))){
						checkhttp = 1;
					}
				}	*/			
			} 
			rbufi++;
			if (rbufi == 3) { 
				rleng = (((int)buf[1])<<8) + (int)buf[2]; 
				//uart_putc(rleng>>8);
				//uart_putc(rleng&255);
				
				if (rleng > sizeof(buf)) 
					rleng = sizeof(buf); 
			} //this is checking the length from packet.
			if (rbufi == 1+2+rleng+1)//check the whole length of packet (add rleng with 7e, 2 byte length, and 1 byte checksum)
			{
					/*for(uint16_t iii = 0;iii<rbufi;iii++){
						uart_putc(buf[iii]);
					} */
				
									
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
/*
					printf("\n-----\n");
					printf("ip=%d.%d.%d.%d\n",rbuf[4],rbuf[5],rbuf[6],rbuf[7]);
					printf("dst_port=%d\n",(((int)rbuf[ 8])<<8) + ((int)rbuf[ 9]));
					printf("src_port=%d\n",(((int)rbuf[10])<<8) + ((int)rbuf[11]));
					printf("proto=%s\n",rbuf[12]?"TCP":"UDP"); //?  is like if statement so it ()?(condition none-0):(condition 0)
					printf("-----\n");
					for(i=14;i<1+2+rleng;i++) printf("%c",rbuf[i]);
					printf("-----\n");
*/
					
						/*for(int temp =i;temp<i+6;temp++){
							uart_putc(buf[temp]);
						}*/
						i--;// Check where HTTP is

						//Need to store important info.
						//buf[0] = 0x20; //Xbee TX IPv4
						//buf[1] = 0x01; //framecnt
						//these two should be taken care of in uart_data()
					 
						 uint32_t dstip = ((uint32_t)buf[4]<<24)+((uint32_t)buf[5]<<16)+((uint32_t)buf[6]<<8)+(uint32_t) buf[7]; //destination IP address to send back
						 uint16_t dstport= ((uint16_t) buf[10]<<8)+((uint16_t)buf[11]); 
						 uint16_t srcport = ((uint16_t) buf[8]<<8) + ((uint16_t)buf[9]);
						//if not cast as int, the shifting will give 0 because it will think it's char(1 bytes)
								
						// all of these should be taken care of in uart_data()
						/*
						//buf[2] = rbuf[4]; buf[3] = rbuf[5]; buf[4] = rbuf[6]; buf[5] = rbuf[7];
						//strcpy should works too.
						*(int *)&buf[2] = *(int *)&rbuf[4]; //copy ip // int* makes &rbuf[4] not only a pointer to 1 byte but to 4 bytes then another * just get the value
						//this might not work in polulu because int in polulu might not be 32 bits=4bytes
						*(short *)&buf[6] = *(short *)&rbuf[10]; //dst port = recv's src port //short*
						*(short *)&buf[8] = *(short *)&rbuf[8]; //src port = recv's dst port //may be 80 decimal, might not be necessary
						*/
						//buf[10] = 1; //0=UDP,1=TCP
						//buf[11] = 2; //|2=close TCP socket after send
						//should be taken care of in uart_data()	
				
					
						//sprintf go to an array, In this case, tbuf
						
						
					if(!memcmp(&buf[14],"GET",3)){
						for(i=20;(i < 1+2+rleng-4) && (memcmp(&buf[i],"HTTP/1",6));i++)
						{;}	
						//char lenbuf[8];
						
						/*static const char PROGMEM lockpage[] ="<html>"
						"<title>Door Lock Control Page</title>"
						"<body>"
						"<h2 align=center>Lock Control</h2><br>"
						"<form method = \"post\" enctype=\"multipart/form-data\" action=\"Control.htm\">"
						"<br><input type = \"submit\" name = \"tolock\" value = \"Lock\"></br>"
						"<br><input type = \"submit\" name = \"tolock\" value = \"Lock\"></br>"
						"</form>"
						"</body>"
						"</html>";*/
						
						if(!memcmp(&buf[17]," / ",3)){
							//i=strlen(homepage);
							itoa(strlen(homepage),lenbuf,10);
					
							strcpy(buf,
							"HTTP/1.1 200 OK\r\n"
							"Server: Ken's g&a XBee-WIFI server\r\n"
							"Content-length: ");
							//uart_putc(buf[67]);			
							strcat(buf,lenbuf);
							strcat(buf,"\r\n"
							"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
							"\r\n");
							//strcat(buf,homepage);
							strcat_P(buf,homepage);
				
							uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
						}/*else if (!memcmp(&buf[17]," /lock",6)){
							//i=strlen(lockpage);
							itoa(strlen(lockpage),lenbuf,10);
						
							strcpy(buf,
							"HTTP/1.1 200 OK\r\n"
							"Server: Ken's g&a XBee-WIFI server\r\n"
							"Content-length: ");
							//uart_putc(buf[67]);
							strcat(buf,lenbuf);
							strcat(buf,"\r\n"
							"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
							"\r\n");
							//strcat(buf,lockpage);
							strcat_P(buf,lockpage);
							
							uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
						}	*/	
					}else if (!memcmp(&buf[14],"POST",4)){
						//WILL NEED TO CREATE CHAR POINTER TO PROGMEM TO SELECT DIFFERENT PAGES, just regular char should be fine?
						
						if(!memcmp(&buf[18]," /credential.htm ", 17)){
							uint16_t ff =0; // use to find a user field
							uint16_t gg =0; // use to find a password field
							for(ff=35;(ff < 1+2+rleng-4) &&(ff<1024)/*length of buffer*/&& (memcmp(&buf[ff],"name=\"user\"\r\n\r\n",15));ff++)
							{;}	
							for(gg=ff;(gg < 1+2+rleng-4) &&(gg<1024)/*length of buffer*/&& (memcmp(&buf[gg],"name=\"passwd\"\r\n\r\n",17));gg++)
							{;}
							
							if(!memcmp(&buf[ff+15],"test\r\n",6)&&!memcmp(&buf[gg+17],"test\r\n",6)){
								itoa(strlen(lockpage),lenbuf,10);
								
								strcpy(buf,
								"HTTP/1.1 200 OK\r\n"
								"Server: Ken's g&a XBee-WIFI server\r\n"
								"Content-length: ");
								//uart_putc(buf[67]);
								strcat(buf,lenbuf);
								strcat(buf,"\r\n"
								"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
								"\r\n");
								//strcat(buf,lockpage);
								strcat_P(buf,lockpage);
								
								uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
							}else{								
								//i=strlen(invalidpage);
								itoa(strlen(invalidpage),lenbuf,10);
						
								strcpy(buf,
								"HTTP/1.1 200 OK\r\n"
								"Server: Ken's g&a XBee-WIFI server\r\n"
								"Content-length: ");
								//uart_putc(buf[67]);
								strcat(buf,lenbuf);
								strcat(buf,"\r\n"
								"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
								"\r\n");
								strcat_P(buf,invalidpage);
						
								uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
							}								
						}else if(!memcmp(&buf[18]," /index.html ", 13)){
							uint16_t ff =0; // use to find a user field
							uint16_t gg =0; // use to find a password field
							for(ff=35;(ff < 1+2+rleng-4) &&(ff<1024)/*length of buffer*/&& (memcmp(&buf[ff],"name=\"user\"\r\n\r\n",15));ff++)
							{;}
							for(gg=ff;(gg < 1+2+rleng-4) &&(gg<1024)/*length of buffer*/&& (memcmp(&buf[gg],"name=\"user\"\r\n\r\n",15));gg++)
							{;}
							
							itoa(strlen(homepage),lenbuf,10);
							strcpy(buf,
								"HTTP/1.1 200 OK\r\n"
								"Server: Ken's g&a XBee-WIFI server\r\n"
								"Content-length: ");
								strcat(buf,lenbuf);
								strcat(buf,"\r\n"
								"Content-type: text/html\r\n"   //text/html, image/gif, image/jpg, text/plain
								"\r\n");
								strcat_P(buf,homepage);
							
							uart_data(buf,(uint16_t) strlen(buf),dstip,dstport,srcport);
						
					}
				}
							
				}
				/*else if (rbuf[3] == 0x88)
				{
					//char 0x7e,0x00,0x05,0x88,0x01,0x49,0x50,0x00,0xdd; //IP ack
					//char 0x7e,0x00,0x05,0x88,0x01,0x43,0x30,0x00,0x03; //C0 ack
				}
				else if (rbuf[3] == 0x89)
				{
					;//censor useless acks
				}
				else if (rbuf[3] == 0x8a)
				{
					//char 0x7e,0x00,0x02,0x8a,0x02,0x73; //modem status update
				}
				else
				{
					printf("\n");
					for(j=0;j<rbufi;j++) printf("%02x ",rbuf[j]);
				}*/
				
			}	
			
		 }
		 
		
	 } 
	
   
}


