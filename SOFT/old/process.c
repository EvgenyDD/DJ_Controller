//#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include "process.h"
#include "indication.h"
#include "HT1621.h"
#include "usb.h"

extern volatile unsigned int T0EN;
extern volatile unsigned char E1dir;

volatile char k=0;


unsigned int timeDiff(unsigned int now, unsigned int before)
/* Вычисляет разность во времени с учётом переполнения счётчика таймера */
{
  return (now >= before) ? (now - before) : ( now + (unsigned int)(65536 - before));
}




volatile signed char jogCount = 0;	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const unsigned char buttonsA[80] PROGMEM = {	1,1,1,1,5,1,1,2,5,0,0,0,0,0,0,5,          
												1,1,1,1,2,1,1,2,5,0,0,0,0,0,0,2,    
												5,1,1,5,2,5,5,5,5,5,0,0,5,5,5,2,
												1,1,1,1,2,1,1,2,2,0,0,0,0,0,0,2,
												1,1,1,1,2,1,1,2,2,0,0,0,0,0,0,2
};
const unsigned char buttonsD[80] PROGMEM = {	21,20,26,24,0,25,22,11,0,21,23,26,24,25,22,  0  ,
												3,14,13,2,4,1,4,12,0,1,13,14,3,4,2,9,    
												0,18,17,0,5,0,0,0,0,0,17,18,0,0,0,8,
												19,15,8,7,7,6,23,13,14,20,15,5,6,7,19,10,
												12,11,10,9,6,5,16,3,2,16,10,11,12,8,9,1
};

const unsigned char encMassA[10] PROGMEM = {	2,2,  2,2,   2,2,  2,2,  0,1};
const unsigned char encMassD[10] PROGMEM = {	14,9, 13,12, 5,3,  6,0,  8,8};

const unsigned char encA[5] PROGMEM = {	1, 1, 2, 2, 3};
const unsigned char encD[5] PROGMEM = {	27,28,27,28,15};

const unsigned char barMass[9] PROGMEM = {		0,2,6,14,30,62,126,254,255};

const unsigned char levelbarMass[30] PROGMEM = {0,1,3,67,83,87,215,223,255,64,80,255};

const unsigned char displayMass[23] PROGMEM = {	207,130,91,211,150, 213,//012345
/* cdhgefba */									221,131,223,215,//6789
												159,220,77,218,93,29,158,//AbCdEFH
												76,152,31,92,206,143//LnPtUП
											  };

const unsigned char faderMass[50] PROGMEM = {  	2,5, 1,1, 1,3, 1,4, 3,13, 1,2, 3,14, 1,5, 3,9, 3,2,
											    3,12, 3,11, 3,7, 3,3, 3,8, 3,1, 3,5, 2,1, 2,3, 2,4,
											 	2,2, 3,10, 3,6, 3,4, 3,16
											 	};	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void ProcessOutBuf(struct OutBufType *OutBuf, struct IOtype *IO)
{


//Faders

	for(unsigned char i=0; i<25; i++) //25
	{ 
		//	i=24;/////////////////////////////////////////////

		if( abs (IO->currFader[i] - IO->lastFader[i]) > 2 )
		{
			//add midi message
			USBAddMsg2(	0x0b, 
						0xb0 + (0x0F & pgm_read_byte(&( faderMass[ i*2 ] ))),
						pgm_read_byte(&( faderMass[ i*2+1 ] )), 	
						IO->currFader[i]);

			//num3(IO->currFader[i],4,5,6);

			IO->lastFader[i] = IO->currFader[i];
		}
	}




//Jogs (10.11...12.13)
//left Jog

	if(BitIsSet(IO->lastEnc, 10) && BitIsSet(IO->lastEnc, 11))
	{

	    if( (BitIsSet(PINB,1) && BitIsReset(PINB,3)) )
		{	//Jog[left] ++ events
		//	jogCount++;
		//	USBAddMsg(0x0b, 0xb1, 30, 0x7F);
		}

		if( (BitIsReset(PINB,1) && BitIsSet(PINB,3)) )
		{	//Jog[left] -- events
		//	jogCount--;
		//	USBAddMsg(0x0b, 0xb1, 30, 0x01);
		}

	}	

	bit_write(BitIsSet(PINB,1),IO->lastEnc, 10);
	bit_write(BitIsSet(PINB,3),IO->lastEnc, 11);


//right Jog

	/*if(E1dir)
		{USBAddMsg(0x0b,0xb2,33, 128-encTime/30);}
	else
		{USBAddMsg(0x0b,0xb2,33, encTime/30);}*/



/*
	if(BitIsSet(IO->lastEnc, 12) && BitIsSet(IO->lastEnc, 13))
	{

		if( (BitIsSet(PINB,0) && BitIsReset(PINB,2)) )
		{	//Jog[right] ++ events
			USBAddMsg(0x0b, 0xb2, 30, 0x7F);
		}

		if( (BitIsReset(PINB,0) && BitIsSet(PINB,2)) )
		{	//Jog[right] -- events
			USBAddMsg(0x0b, 0xb2, 30, 0x01);
		}
		
	}

	bit_write(BitIsSet(PINB,0),IO->lastEnc, 12);
	bit_write(BitIsSet(PINB,2),IO->lastEnc, 13);
*/



//Encoders
		
	for(unsigned char i=0; i<10;  i++) 
	{
		bit_write( 	BitIsSet(			IO->buttons[	pgm_read_byte(&(encMassA[i]))	],
										pgm_read_byte(&(encMassD[i]))						), 
									//выборка нужного бита из массива кнопок
					IO->currEnc, i  //куда и [номер] бит записать
		);
	} 

	for(unsigned char i=0; i<10; i += 2)
	{
		if(BitIsSet(IO->lastEnc, i) && BitIsSet(IO->lastEnc, i+1))
		{

			if( (BitIsSet(IO->currEnc, i) && BitIsReset(IO->currEnc, i+1)) )
			{	//Encoder[i] ++ events
				USBAddMsg(	0x0B, 
							0xB0 + (0x0F & pgm_read_byte(&( encA[ i/2 ] ))),
							pgm_read_byte(&( encD[ i/2 ] )), 	
							0x01);

/*хуня*/

	IO->fuck++;
/*хуня*/
									
			}

			if( (BitIsReset(IO->currEnc, i) && BitIsSet(IO->currEnc, i+1)) )
			{	//Encoder[i] -- events
				USBAddMsg(	0x0B, 
							0xB0 + (0x0F & pgm_read_byte(&( encA[ i/2 ] ))),
							pgm_read_byte(&( encD[ i/2 ] )), 	
							0x7F);

/*хуня*/

IO->fuck--;
/*хуня*/
			}			
		}

		bit_write(BitIsSet(IO->currEnc, i),IO->lastEnc, i);
		bit_write(BitIsSet(IO->currEnc, i+1),IO->lastEnc, i+1);
	}
	

//7segments
//все вынесено в main
/*	IO->light[15] = pgm_read_byte(&( displayMass[ OutBuf->digit[0] ] ));//1
	IO->light[16] = pgm_read_byte(&( displayMass[ OutBuf->digit[2] ] ));//3
	IO->light[17] = pgm_read_byte(&( displayMass[ OutBuf->digit[1] ] ));//2
	IO->light[18] = pgm_read_byte(&( displayMass[ OutBuf->digit[3] ] ));//4
*/




//Progress Bar

unsigned char bl, br;
#define THRESHOLD 	15
		if(IO->Flash >0 && OutBuf->barLeft > THRESHOLD) bl = 0;
		else bl = OutBuf->barLeft;

		if(IO->Flash >0 && OutBuf->barRight > THRESHOLD) br = 0;
		else br = OutBuf->barRight;

	if( bl > 8 ) IO->light[7] = 255;	//1	
	else IO->light[7] = pgm_read_byte(&( barMass[ bl  ]	));

	if( bl < 9) IO->light[5] = 0;
	else
	{
		if( bl > 16) IO->light[5] = 255;   //2
		else IO->light[5] = pgm_read_byte(&( barMass[ bl - 8] )); 
	}
	
unsigned char s1, s2;
	if( bl < 17) s1 = 0; 				//3 - half
	else s1 = pgm_read_byte(&( barMass[ bl - 16]));
		
	if( br > 4) s2 = 225;
	else
	{
		if( br == 0) s2 = 0;
		else s2 = pgm_read_byte(&( barMass[ br + 4]	)) - 30;
	}
	IO->light[6] = s1 | s2;


	if( br < 5) IO->light[9] = 0;		//4
	else
	{
		if( br > 12) IO->light[9] = 255;
		else IO->light[9] = pgm_read_byte(&( barMass[ br - 4] ));
	} 

	if( br < 13) IO->light[8] = 0;  	//5
	else IO->light[8] = pgm_read_byte(&( barMass[ br - 12]));





//Volume Bar
	
	if( OutBuf->levelLeft > 8) {IO->light[10] = 255;}
	else {IO->light[10] = pgm_read_byte(&( levelbarMass[ OutBuf->levelLeft ] ));}

	if( OutBuf->levelLeft < 9) s1 = 0;
	else s1 = pgm_read_byte(&( levelbarMass[ OutBuf->levelLeft - 8 ] ));
	if( OutBuf->levelRight < 9) s2 = 0;
	else s2 = pgm_read_byte(&( levelbarMass[ OutBuf->levelRight ] ));
	IO->light[12] = s1 | s2;

	if( OutBuf->levelRight > 8) {IO->light[11] = 255;}
	else {IO->light[11] = pgm_read_byte(&( levelbarMass[ OutBuf->levelRight ] ));}





/*--------------------------- Backlight --------------------------------------*/
	//	0-39   111-119

	unsigned char n=0;
	for(unsigned char i = 0; i<120; i++)
	{
		n++;//0-48
		if(i==40) i=111;
		unsigned char A = BitIsSet(IO->lightIN[n>>3], n-(n>>3)*8);
		unsigned char B = BitIsSet(IO->lightFL[n>>3], n-(n>>3)*8);
		if(A && B)
		{
			if(IO->Flash) IO->light[i>>3] |= (1<<  (i-(i>>3)*8)   );
			else IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );
		}
		if(!A && B)
		{	
			if(IO->Blink) IO->light[i>>3] |= (1<<  (i-(i>>3)*8)   );
			else IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );
		}
		if(A && !B)
		{
			IO->light[i>>3] |= (1<<  (i-(i>>3)*8)   );
		}
		if(!A && !B)
		{
			IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );
		}

	}

/*============================================================================*/
/*=========================== BUTTONS ========================================*/
/*============================================================================*/


	for(unsigned char i = 0; i<80; i++)
	{
		while(
				i == 4 	||
				i == 8 	||
				i == 24 ||
				i == 32	||
				i == 35	||
				i == 37	||
				i == 38	||
				i == 39	||
				i == 40 ||
				i == 41 ||
				i == 44 ||
				i == 45 ||
				i == 46 	) i++;

		if( BitIsReset(IO->lastButtons[i/16], (i%16) ) &&  BitIsSet(IO->buttons[i/16],	(i%16) ) )
		{	//events btns
			
			switch(i)
			{
/*			case 48:
				break;
			case 62:
				break;
*/			
			default:
	   			//WriteDigit_1621(1,i/10,0); 
	  			//WriteDigit_1621(2,i%10,1);
			
				USBAddMsg(	0x09, 
							0x90 + (0x0F & pgm_read_byte(&( buttonsA[ i ] ))),
							pgm_read_byte(&( buttonsD[ i ] )), 	
							0x7F);
				k = 1;
				break;
			}
		}
		if( BitIsSet(IO->lastButtons[i/16], (i%16) ) &&  BitIsReset(IO->buttons[i/16],	(i%16) ) )
		{	//events btns
			
			switch(i)
			{
/*			case 48: //left
				break;
			case 62:
				break;*/

			default:	
	   		//	WriteDigit_1621(1,i/10,0); 
	  		//	WriteDigit_1621(2,i%10,1);

				USBAddMsg(	0x08, 
							0x80 + (0x0F & pgm_read_byte(&( buttonsA[ i ] ))),
							pgm_read_byte(&( buttonsD[ i ] )), 	
							0x7F);
				break;
			}
		}
	}

	for(unsigned char i=0; i<5;  i++) 
		IO->lastButtons[i] = IO->buttons[i]; 

}


/* INTERRUPTS */

/*
ISR(INT1_vect) 
{	
sei();

}
*/


ISR(INT2_vect)
/* RIght Jog */ 
{	
	if(BitIsSet(PINB, 0))
		jogCount++;		
	else	
		jogCount--;
}


ISR(TIMER0_COMP_vect/*TIMER0_OVF_vect*/)
/* timer 0 overflow interrupt */
{	
	static unsigned char countTimer=0;
	static unsigned char lastCount = 100;
	//PORTA ^= (1<<2);

	//if(++countTimer >=10)
	//{
	if(jogCount!=0) 
	{
		if(jogCount > 63) jogCount = 63;
		if(jogCount < -63) jogCount = -63;

		USBAddMsg(0x0b, 0xb1, 30, 64+jogCount);

		if(lastCount == 100) 
		{
			USBAddMsg(0x09, 0x84, 0, 0x7F);
			lastCount = 0;
		}
	}
	else 
	{		
		if(lastCount != 100) lastCount++;
	}


	if(lastCount >= (20 /4) && lastCount < 100) 
	{
		USBAddMsg(0x08, 0x84, 0, 0x7F);
		lastCount = 100;
	}

	jogCount = 0;


		//countTimer = 0;
		//PORTA ^= (1<<4);
	//}

/*
	static unsigned int h = 101;

	if(h && k) h--;
	
//	PORTA ^= (1<<4);

	if(h<= 100 && h)
	{
		USBAddMsg(0x0b, 0xb1, 30, 68);	
		//USBAddMsg2(0x0b, 0xb1, 30, 68);
	//	PORTA ^= (1<<2);
	}

	if(!h) {k = 0; h = 101;}
*/

}

