/* Includes ------------------------------------------------------------------*/
//#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include "indication.h"
#include "process.h"
#include "HT1621.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private constaints --------------------------------------------------------*/
const uint8_t buttonsA[80] PROGMEM = {	2,2,1,1,6,1,2,1,6,2,2,0,0,0,2,6,          
										1,1,1,1,2,1,1,1,6,0,0,0,0,0,0,0,    
										6,1,1,6,2,6,6,6,6,6,0,0,6,6,6,0,
										1,1,1,1,2,1,2,1,2,2,0,0,0,0,0,0,
										1,1,1,1,2,1,1,1,2,0,0,0,0,0,0,0
									 }; 
const uint8_t buttonsD[80] PROGMEM = {	13,12,26,24,0,25,14,28, 0, 9,11,26,24,25,10, 0,
										3, 14,13,2, 4, 1, 4,29, 0, 1,13,14, 3, 4, 2,29,    
										0, 18,17,0, 5, 0, 0, 0, 0, 0,17,18, 0, 0, 0,28,
										19,15,8, 7, 7, 6,15,30,16, 8,15, 5, 6, 7,19,30,
										12,11,10,9, 6, 5,16,31, 2,16,10,11,12, 8, 9,31
									 };

const uint8_t encMassA[10] PROGMEM = {2,2,  2,2,   2,2,  2,2,  0,1};
const uint8_t encMassD[10] PROGMEM = {14,9, 13,12, 5,3,  6,0,  8,8};

const uint8_t encA[5] PROGMEM = {0, 0, 1, 1,  2};
const uint8_t encD[5] PROGMEM = {8, 9, 8, 9, 11};

const uint8_t barMass[9] PROGMEM = {0,2,6,14,30,62,126,254,255};

const uint8_t levelbarMass[30] PROGMEM = {0,1,3,67,83,87,215,223,255,64,80,255};

const uint8_t displayMass[23] PROGMEM = {	207,130,91,211,150, 213,//012345
/* cdhgefba */								221,131,223,215,//6789
											159,220,77,218,93,29,158,//AbCdEFH
											76,152,31,92,206,143//LnPtUП
									    };

const uint8_t faderMass[50] PROGMEM = {  	1,5, 2,3, 2,5, 2,6, 0,6, 2,4, 1,6, 0,5, 2,1, 0,2,
											1,4, 0,4, 2,2, 0,3, 0,0, 0,1, 1,2, 2,7, 2,9, 2,10,
											2,8, 1,0, 1,3, 1,1, 2,0
									  };


/* Private variables ---------------------------------------------------------*/
volatile signed char jogCount[2] = {0,0};	


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : SendFaders
* Description    : Send values of all faders to initialize the Traktor
*******************************************************************************/
void SendFaders(struct IOtype *IO)
{
	for(uint8_t i=0; i<25; i++) 
	{ 
		MidiAddMsg(	0xb0 + (0x0F & pgm_read_byte(&( faderMass[ i*2 ] ))),
					pgm_read_byte(&( faderMass[ i*2+1 ] )), 	
					IO->currFader[i]);
	}
}


/*******************************************************************************
* Function Name  : ProcessOutBuf
* Description    : Handle filling midi CB and Output HAL structures
* Input          : pointer to OutBuf & IO structures
*******************************************************************************/
void ProcessOutBuf(struct OutBufType *OutBuf, struct IOtype *IO)
{

//Faders

	for(uint8_t i=0; i<25; i++) //25
	{ 
		//	i=24;/////////////////////////////////////////////

		if( abs (IO->currFader[i] - IO->lastFader[i]) > 2 )
		{
			//add midi message
			MidiAddMsg(	0xb0 + (0x0F & pgm_read_byte(&( faderMass[ i*2 ] ))),
						pgm_read_byte(&( faderMass[ i*2+1 ] )), 	
						IO->currFader[i]);

			//num3(IO->currFader[i],4,5,6);

			IO->lastFader[i] = IO->currFader[i];
		}
	}


//Encoders		
	for(uint8_t i=0; i<10;  i++) 
	{
		BitWrite( 	BitIsSet(	IO->buttons[	pgm_read_byte(&(encMassA[i]))	],
								pgm_read_byte(&(encMassD[i])) ), 
									//выборка нужного бита из массива кнопок
					IO->currEnc, i  //куда и [номер] бит записать
		);
	} 

	for(uint8_t i=0; i<10; i += 2)
	{
		if(BitIsSet(IO->lastEnc, i) && BitIsSet(IO->lastEnc, i+1))
		{

			if( (BitIsSet(IO->currEnc, i) && BitIsReset(IO->currEnc, i+1)) )
			{	//Encoder[i] ++ events
				MidiAddMsg(	0xB0 + (0x0F & pgm_read_byte(&( encA[ i/2 ] ))),
							pgm_read_byte(&( encD[ i/2 ] )), 	
							0x01);									
			}

			if( (BitIsReset(IO->currEnc, i) && BitIsSet(IO->currEnc, i+1)) )
			{	//Encoder[i] -- events
				MidiAddMsg(	0xB0 + (0x0F & pgm_read_byte(&( encA[ i/2 ] ))),
							pgm_read_byte(&( encD[ i/2 ] )), 	
							0x7F);
			}			
		}

		BitWrite(BitIsSet(IO->currEnc, i),IO->lastEnc, i);
		BitWrite(BitIsSet(IO->currEnc, i+1),IO->lastEnc, i+1);
	}
	

//7segments
//moved to main
#if 0
	IO->light[15] = pgm_read_byte(&( displayMass[ OutBuf->digit[0] ] ));//1
	IO->light[16] = pgm_read_byte(&( displayMass[ OutBuf->digit[2] ] ));//3
	IO->light[17] = pgm_read_byte(&( displayMass[ OutBuf->digit[1] ] ));//2
	IO->light[18] = pgm_read_byte(&( displayMass[ OutBuf->digit[3] ] ));//4
#endif



//Progress Bar
uint8_t bl, br;

#define THRESHOLD 	15

	if(IO->Flash >0 && OutBuf->barLeft > THRESHOLD) 
		bl = 0;
	else 
		bl = OutBuf->barLeft;

	if(IO->Flash >0 && OutBuf->barRight > THRESHOLD)
		br = 0;
	else 
		br = OutBuf->barRight;


	if( bl > 8 ) 		IO->light[7] = 255;	//1	
	else 				IO->light[7] = pgm_read_byte(&( barMass[ bl  ]	));

	if( bl < 9) 		IO->light[5] = 0;
	else
	{
		if( bl > 16) 	IO->light[5] = 255;   //2
		else 			IO->light[5] = pgm_read_byte(&( barMass[ bl - 8] )); 
	}
	

uint8_t s1, s2;
	if( bl < 17) 	s1 = 0; 				//3 - half
	else 			s1 = pgm_read_byte(&( barMass[ bl - 16]));
		
	if( br > 4) 	s2 = 225;
	else
	{
		if( br == 0)s2 = 0;
		else 		s2 = pgm_read_byte(&( barMass[ br + 4]	)) - 30;
	}
	IO->light[6] = s1 | s2;


	if( br < 5) 	IO->light[9] = 0;		//4
	else
	{
		if( br > 12)IO->light[9] = 255;
		else 		IO->light[9] = pgm_read_byte(&( barMass[ br - 4] ));
	} 

	if( br < 13) 	IO->light[8] = 0;  	//5
	else 			IO->light[8] = pgm_read_byte(&( barMass[ br - 12]));


//Volume Bar	
	if( OutBuf->levelLeft > 8) 
		IO->light[10] = 255;
	else 
		IO->light[10] = pgm_read_byte(&( levelbarMass[ OutBuf->levelLeft ] ));


	if( OutBuf->levelLeft < 9) 
		s1 = 0;
	else 
		s1 = pgm_read_byte(&( levelbarMass[ OutBuf->levelLeft - 8 ] ));
	

	if( OutBuf->levelRight < 9) 
		s2 = 0;
	else 
		s2 = pgm_read_byte(&( levelbarMass[ OutBuf->levelRight ] ));


	IO->light[12] = s1 | s2;


	if( OutBuf->levelRight > 8) 
		IO->light[11] = 255;
	else 
		IO->light[11] = pgm_read_byte(&( levelbarMass[ OutBuf->levelRight ] ));


/*--------------------------- Backlight --------------------------------------*/
	//	0-39   111-119

	uint8_t n=0;

	for(uint8_t i = 0; i<120; i++)
	{
		n++;//0-48
		if(i==40) i=111;

		uint8_t A = BitIsSet(IO->lightIN[n>>3], n-(n>>3)*8);
		uint8_t B = BitIsSet(IO->lightFL[n>>3], n-(n>>3)*8);

		if(A && B)
		{
			if(IO->Flash) IO->light[i>>3] |=  (1<<  (i-(i>>3)*8)   );
			else 		  IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );
		}

		if(!A && B)
		{	
			if(IO->Blink)IO->light[i>>3] |=  (1<<  (i-(i>>3)*8)   );
			else 		 IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );
		}

		if(A && !B)		IO->light[i>>3] |=  (1<<  (i-(i>>3)*8)   );

		if(!A && !B)	IO->light[i>>3] &= ~(1<<  (i-(i>>3)*8)   );

	}

/*============================================================================*/
/*=========================== BUTTONS ========================================*/
/*============================================================================*/
	for(uint8_t i = 0; i<80; i++)
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
		{	//event: button on			
			MidiAddMsg(	0x90 + (0x0F & pgm_read_byte(&( buttonsA[ i ] ))),
						pgm_read_byte(&( buttonsD[ i ] )), 	
						MIDI_MAX);

		}

		if( BitIsSet(IO->lastButtons[i/16], (i%16) ) &&  BitIsReset(IO->buttons[i/16],	(i%16) ) )
		{	//event: button off
			MidiAddMsg(	0x80 + (0x0F & pgm_read_byte(&( buttonsA[ i ] ))),
						pgm_read_byte(&( buttonsD[ i ] )), 	
						MIDI_MAX);
		}
	}

	for(uint8_t i=0; i<5;  i++) 
		IO->lastButtons[i] = IO->buttons[i]; 

}


/*******************************************************************************
* Function Name  : INT1_vector
* Description    : INT1 Interrupt Handler (Left Jog)
*******************************************************************************/
ISR(INT1_vect) 
{	
	if(BitIsReset(PINB, 1))
		jogCount[0]++;		
	else	
		jogCount[0]--;

	GIFR &= ~(1<<INTF1);
}


/*******************************************************************************
* Function Name  : INT2_vector
* Description    : INT2 Interrupt Handler (Right Jog)
*******************************************************************************/
ISR(INT2_vect)
{	
	if(BitIsSet(PINB, 0))
		jogCount[1]++;		
	else	
		jogCount[1]--;

	GIFR &= ~(1<<INTF2);
}


/*******************************************************************************
* Function Name  : TIMER0_COMP_vector
* Description    : TIMER0 Compare Interrupt Handler - Jogs processing
*******************************************************************************/
ISR(TIMER0_COMP_vect)
{	
#define STOP 100
	static uint8_t lastCount[2] = {STOP,STOP};

	for(uint8_t i=0; i<2; i++)
	{
		if(jogCount[i]!=0) 
		{
			if(jogCount[i] > 63) 	jogCount[i] = 63;
			if(jogCount[i] < -63) 	jogCount[i] = -63;


			if(i==0) MidiAddMsg(0xb0, 7, 64+jogCount[i]);
	    	else 	 MidiAddMsg(0xb1, 7, 64+jogCount[i]);

			if(lastCount[i] == STOP)	lastCount[i] = 0;
		}
		else 
		{		
			if(lastCount[i] != STOP) lastCount[i]++;
		}


		if(lastCount[i] >= (20 /4) && lastCount[i] < STOP) 
					lastCount[i] = STOP;

		jogCount[i] = 0;
	}
}
