/*
* 	MIDI DJ CONTROLLER USB 
*/

/* Includes ------------------------------------------------------------------*/
//#include <string.h>
//#include <stdio.h>
//#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay_basic.h>

#include "indication.h"
#include "process.h"
#include "HT1621.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//Circular buffer size
#define cbOut_MAX 	35
#define cbIn_MAX 	35

//#define LCDDEBUG


/* Private macro -------------------------------------------------------------*/
/* Private constaints --------------------------------------------------------*/
	//0 in code - MIDI channel 1 on computer
	//1 in code - MIDI channel 2 on computer...
const uint8_t BL[3][32] PROGMEM = {							  //-V--V--V--V--reserved in line 0 & 1    
{15,50,50,50,50,24,23,22,29, 1,  32,31,30, 8, 7,50,50,50,50,14,  50,50,50,50,13, 9,10,50,40,39,  38,33},
{44,50,50,50,50,28,19,18,17, 1,  27,26,25, 2, 1,50,50,50,50,43,  50,50,50,50,48,49,41,50,35,36,  37,11},
{50,50,12,50, 6, 3, 5, 4,16,21,  20,34,47,46,45,42,50,50,50,50,  50,50,50,50,50,50,50,50,50,50,  50,50}};


const uint8_t display[22] PROGMEM = {	243,91,  162,221,  32,223,  32,150,  32,91,
										0,130,   0,91,     0,150,   0,223,  130,221,  211,91};

const uint8_t RGBmass[3][7] PROGMEM = { {0, 0,   255, 255, 255,	255, 0	},
										{0, 0,   50,  120, 150, 200, 255},
										{0, 255, 100, 0,   0,   200, 0  }};


/* Private variables ---------------------------------------------------------*/
struct IOtype IO; 
struct OutBufType OutBuf;

uint8_t cbOutStart = 0, cbOutEnd = 0;
uint8_t cbInStart = 0, cbInEnd = 0;
volatile struct cbType cbOut[cbOut_MAX], cbIn[cbIn_MAX];

volatile uint8_t midiMessage[3];
volatile uint8_t TxFlag=0;

uint8_t DDLeft=0, DDRight=0;


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MidiIn
* Description    : Receive information from MIDI HOST
*******************************************************************************/
void UARTProcessIn()
{
	if(cbInStart != cbInEnd)
	{
		static uint8_t g = 0;
		if(g!= 10)
		{
		//	ResetIO(&IO);
			if(++g == 10) SendFaders(&IO);
		}

	  	uint8_t b,c,d;

		b = cbIn[cbInStart].channel;
		c = cbIn[cbInStart].control;
		d = cbIn[cbInStart].value;

		if(++cbInStart == cbIn_MAX) cbInStart = 0;


		if(((b & 0x0F) < 3) && c < 35)
		{
			//note off
			if(d == 0)		
			{
				uint8_t n = pgm_read_byte(&(BL[b&0x0F][c]));

				if(n<50)
				{
					BitWrite( 	0, 
								IO.lightIN[n>>3], 
								n-(n>>3)*8 ); 
					BitWrite( 	0, 
								IO.lightFL[n>>3], 
								n-(n>>3)*8 ); 
				} 
			}

			//note #blink#
			else if(d == 32)	
			{
				uint8_t n = pgm_read_byte(&(BL[b&0x0F][c]));

				if(n<50)
				{
					BitWrite( 	0, 
								IO.lightIN[n>>3], 
								n-(n>>3)*8 );
					BitWrite( 	1, 
								IO.lightFL[n>>3], 
								n-(n>>3)*8 );
				} 
			}

			//note #flash#
			else if(d == 64)		
			{
				uint8_t n = pgm_read_byte(&(BL[b&0x0F][c]));

				if(n<50)
				{
					BitWrite( 	1, 
								IO.lightIN[n>>3], 
								n-(n>>3)*8 );
					BitWrite( 	1, 
								IO.lightFL[n>>3], 
								n-(n>>3)*8 );
				}
			}

			//note on
			else /*if(d == 127)	*/
			{
				uint8_t n = pgm_read_byte(&(BL[b&0x0F][c]));

				if(n<50)
				{
					BitWrite( 	1, 
								IO.lightIN[n>>3], 
								n-(n>>3)*8 );  
					BitWrite( 	0, 
								IO.lightFL[n>>3], 
								n-(n>>3)*8 ); 
				}
			}
		}

		//other
		if(((b & 0x0F) == 3) ) //ch4
		{
			if(c == 0 || c == 2) 	//A & C progress bar
				OutBuf.barLeft = d%20;

			if(c == 1 || c == 3) 	//B & D progress bar
				OutBuf.barRight = d%20;

			if(c == 4 || c == 6)  	//A & C 7seg
			{
				IO.light[15] = pgm_read_byte(&( display[ (d%11)*2  ] ));
				IO.light[17] = pgm_read_byte(&( display[ (d%11)*2+1 ]));
			}

			if(c == 5 || c == 7) 	//B & D 7seg
			{
				IO.light[16] = pgm_read_byte(&( display[ (d%11)*2 ] ));
				IO.light[18] = pgm_read_byte(&( display[ (d%11)*2+1 ]));
			}

			if(c == 8 || c == 10) 	//A & C volume bar
				OutBuf.levelLeft  = d%10;

			if(c == 9 || c == 11)	//B & D volume bar
				OutBuf.levelRight = d%10;
		}

		//RGB
		if(((b & 0x0F) == 4) || ((b & 0x0F) == 5) ) //ch5 & ch6
		{
			IO.RGB[c].R = pgm_read_byte(&(RGBmass[0][d%7]));
			IO.RGB[c].G = pgm_read_byte(&(RGBmass[1][d%7]));
			IO.RGB[c].B = pgm_read_byte(&(RGBmass[2][d%7]));
		}
	

	#ifdef LCDDEBUG
	uint8_t y, a=cbInEnd;

	    y=a/100;
	   	WriteDigit_1621(1,y,0);
	    y=a%100;
	    y=y/10;
	   	WriteDigit_1621(2,y,0); 
	    y=a%10;
	   	WriteDigit_1621(3,y,1);

	    y=b/100;
	   	WriteDigit_1621(4,y,0);
	    y=b%100;
	    y=y/10;
	   	WriteDigit_1621(5,y,0); 
	    y=b%10;
	   	WriteDigit_1621(6,y,1);

	    y=c/100;
	   	WriteDigit_1621(7,y,0);
	    y=c%100;
	    y=y/10;
	   	WriteDigit_1621(8,y,0); 
	    y=c%10;
	   	WriteDigit_1621(9,y,1);

	    y=d/100;
	   	WriteDigit_1621(10,y,0);
	    y=d%100;
	    y=y/10;
	   	WriteDigit_1621(11,y,0); 
	    y=d%10;
	   	WriteDigit_1621(12,y,1);
	#endif

	}
}


/*******************************************************************************
* Function Name  : UARTProcessOut
* Description    : Add message from circular buffer to Midi out interface
*******************************************************************************/
void UARTProcessOut()
{
	if( cbOutStart != cbOutEnd )
	{	
		if(TxFlag == 0) //if 3 bytes were sent
		{
    		if(++cbOutStart == cbOut_MAX) cbOutStart = 0;
    
    		midiMessage[0] = cbOut[cbOutStart].channel;
    		midiMessage[1] = cbOut[cbOutStart].control;
    		midiMessage[2] = cbOut[cbOutStart].value;
    		TxFlag = 1;

    		UDR = midiMessage[0];
        	UCSRB |= (1<<UDRIE);
		}			
	}
}


/*******************************************************************************
* Function Name  : MidiAddMsg
* Description    : Add message to the Midi out circular buffer
* Input			 : Midi message type, channel and value
*******************************************************************************/
void MidiAddMsg(uint8_t channel, uint8_t control, uint8_t value)
{
	uint8_t temp = channel & 0xF0;	//remember the ADRESS of midi message

	if(control == 19)	//if we press 1 of 2 deck buttons->change deck
	{
		if(channel == 0x90) 
		{
			if(DDLeft==1) 	
			{
				DDLeft = 0; 

				BitReset(UCSRB, RXCIE);
				cbIn[cbInEnd].channel	= 0x90; 
				cbIn[cbInEnd].control	= 0; 
				cbIn[cbInEnd].value		= MIDI_NULL; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;

				cbIn[cbInEnd].channel	= 0x90; 
				cbIn[cbInEnd].control	= 19; 
				cbIn[cbInEnd].value		= MIDI_MAX; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
				BitSet(UCSRB, RXCIE);

				if(++cbOutEnd == cbOut_MAX) cbOutEnd = 0;
				cbOut[cbOutEnd].channel = 0x90;
				cbOut[cbOutEnd].control = 19;
				cbOut[cbOutEnd].value 	= MIDI_NULL; 
			}
			else 		
			{
				DDLeft = 1;

				BitReset(UCSRB, RXCIE);
				cbIn[cbInEnd].channel	= 0x90; 
				cbIn[cbInEnd].control	= 0; 
				cbIn[cbInEnd].value		= MIDI_MAX; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;

				cbIn[cbInEnd].channel	= 0x90; 
				cbIn[cbInEnd].control	= 19; 
				cbIn[cbInEnd].value		= MIDI_NULL; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
				BitSet(UCSRB, RXCIE);

				if(++cbOutEnd == cbOut_MAX) cbOutEnd = 0;
				cbOut[cbOutEnd].channel = 0x90;
				cbOut[cbOutEnd].control = 19;
				cbOut[cbOutEnd].value 	= MIDI_MAX;
			}
		}

		if(channel == 0x91)
		{	
			if(DDRight==1) 
			{
				DDRight = 0;

				BitReset(UCSRB, RXCIE);
				cbIn[cbInEnd].channel	= 0x91; 
				cbIn[cbInEnd].control	= 0; 
				cbIn[cbInEnd].value		= MIDI_NULL; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;

				cbIn[cbInEnd].channel	= 0x91; 
				cbIn[cbInEnd].control	= 19; 
				cbIn[cbInEnd].value		= MIDI_MAX; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
				BitSet(UCSRB, RXCIE);

				if(++cbOutEnd == cbOut_MAX) cbOutEnd = 0;
				cbOut[cbOutEnd].channel = 0x91;
				cbOut[cbOutEnd].control = 19;
				cbOut[cbOutEnd].value 	= MIDI_NULL;
			}
			else 		
			{
				DDRight = 1;

				BitReset(UCSRB, RXCIE);
				cbIn[cbInEnd].channel	= 0x91; 
				cbIn[cbInEnd].control	= 0; 
				cbIn[cbInEnd].value		= MIDI_MAX; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;

				cbIn[cbInEnd].channel	= 0x91; 
				cbIn[cbInEnd].control	= 19; 
				cbIn[cbInEnd].value		= MIDI_NULL; 
				if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
				BitSet(UCSRB, RXCIE);

				if(++cbOutEnd == cbOut_MAX) cbOutEnd = 0;
				cbOut[cbOutEnd].channel = 0x91;
				cbOut[cbOutEnd].control = 19;
				cbOut[cbOutEnd].value 	= MIDI_MAX;
			}
		}	
	}	
	else	//if we press/move/touch other controls => sends message to TRAKTOR		
	{
		if(((channel & 0x0F) == 0) && DDLeft  && control != 9) 	channel = 3; 
		if(((channel & 0x0F) == 1) && DDRight && control != 9) 	channel = 4;

		channel |= temp;	//return back: type + ADRESS

		if(++cbOutEnd == cbOut_MAX) cbOutEnd = 0;
		cbOut[cbOutEnd].channel = channel;
		cbOut[cbOutEnd].control = control;
		cbOut[cbOutEnd].value 	= value;

		UARTProcessOut();
	}
}


/*******************************************************************************
* Function Name  : HardwareInit
* Description    : Initialize peripheral
*******************************************************************************/
void HardwareInit(void)
{
//I/O
	DDRA 	= 0b11010100;
	DDRB 	= 0b10111000;
	DDRC 	= 0b11111111;
	DDRD    = 0b11110111;

//ADC 
    ADMUX 	= (0<<REFS1)|(1<<REFS0)|(1<<ADLAR) | (0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);
    ADCSRA 	= (1<<ADEN)|(0<<ADSC)|(0<<ADATE)|(1<<ADIE) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); 
    SFIOR 	= 0;

//IRQ
	GICR 	|= /*(1<<INT0)|*/(1<<INT1)|(1<<INT2);
    MCUCR 	|= /*(1<<ISC01)|(0<<ISC00)|*/(1<<ISC11)|(1<<ISC10);
    MCUCSR 	|= (1<<ISC2);
	
//SPI
	SPCR 	= (1<<SPE)|(1<<MSTR)|(1<<SPR0);

//PWM
    TCCR2 	|= (1<<WGM20)|(1<<WGM21)|(0<<COM20)|(1<<COM21)|(1<<CS20);
    TCCR1A 	|= (1<<COM1A1)|(1<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(1<<WGM10)|(0<<WGM11);
    TCCR1B 	|= (1<<WGM12)|(0<<WGM13)|(0<<CS12)|(0<<CS11)|(1<<CS10); 
	TCCR0 	|= (1<<WGM01)|(1<<CS02)|(0<<CS01)|(1<<CS00);
	TIMSK 	|= (1<<OCIE0)|(1<<TOIE0);

	OCR0 = 30;//250 for 12.8ms      77 for 4ms

//USART
	#define BAUD_PRESCALE 39
	UBRRL = (uint8_t)BAUD_PRESCALE;// Load lower 8-bits into the low byte of the UBRR register
    UBRRH = (uint8_t)(BAUD_PRESCALE >> 8); 
    UCSRB = (1<<TXEN) |(1<<RXEN) |(1<<RXCIE);
	UCSRC = (1<<URSEL)|(1<<UCSZ0)|(1<<UCSZ1);

//watchdog
	wdt_enable(WDTO_1S);

	cbIn[cbInEnd].channel=0x90; 
	cbIn[cbInEnd].control=19; 
	cbIn[cbInEnd].value=127; 
	if(++cbInEnd == cbIn_MAX) cbInEnd = 0;

	cbIn[cbInEnd].channel=0x91; 
	cbIn[cbInEnd].control=19; 
	cbIn[cbInEnd].value=127; 
	if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
} 


/*******************************************************************************
* Function Name  : main
* Description    : Main routine
*******************************************************************************/
int main(void)
{
	HardwareInit();
	ResetIO(&IO);

#ifdef LCDDEBUG
	SendCmd(BIAS); //setup bias and working period 
	SendCmd(SYSEN); //start system oscillator 
	SendCmd(LCDON); //switch on LCD bias generator 
	SendCmd(TONE2K);
#endif

	sei();					//enable interrupts
	ADCSRA |= (1<<ADSC);	//start ADC


	for (;;) 	
	{		
		wdt_reset();
		
		ProcessIO(&IO);
		ProcessOutBuf(&OutBuf, &IO);			

		UARTProcessIn();
		UARTProcessOut();


		//BLINK
		static uint16_t R=0;

		if(++R >= 80)  
		{ 
			if( IO.Flash == 0 ) IO.Flash = 255;
			else 				IO.Flash = 0;
		}

		if(R == 160) 			IO.Blink = 255;

		if(R == 170) 
		{
			R = 0; 
			IO.Blink = 0; 
		}
	
	
		//JOG TOUCH	
		static uint8_t touchKey = 0;
		uint8_t touchCnt[2] = {0,0};

		for(uint8_t i=0; i<50; i++)
		{
			//Right jog touch
			BitSet(DDRD, 6);
			BitReset(PORTD, 6);  
			BitReset(DDRD, 6); 
			//BitSet(PORTD, 6); //comment if use 1MOhm resistor pull up

			//Left Jog touch
			BitSet(DDRD, 2);
			BitReset(PORTD, 2);  
			BitReset(DDRD, 2); 
			//BitSet(PORTD, 2); //comment if use 1MOhm resistor pull up

			_delay_loop_2(12);//15

			if(BitIsReset(PIND, 6)) touchCnt[0]++;
			if(BitIsReset(PIND, 2)) touchCnt[1]++;
		}
    
	    if(touchCnt[0])
		{
			if(BitIsReset(touchKey, 0))
			{
				BitSet(touchKey, 0);
				MidiAddMsg(0x91, 32, MIDI_MAX);
			}
		}             
	    else                    
	    {
			if(BitIsSet(touchKey, 0))
			{
				BitReset(touchKey, 0);
				MidiAddMsg(0x81, 32, MIDI_MAX);
			}
		}


	    if(touchCnt[1])
		{
			if(BitIsReset(touchKey, 1))
			{
				BitSet(touchKey, 1);
				MidiAddMsg(0x90, 32, MIDI_MAX);
			}
		}             
	    else                    
	    {
			if(BitIsSet(touchKey, 1))
			{
				BitReset(touchKey, 1);
				MidiAddMsg(0x80, 32, MIDI_MAX);
			}
		}
	}

	return 0; 
}


/*******************************************************************************
* Function Name  : USART_RXC_vect
* Description    : UART Receive interrupt routine
*******************************************************************************/
ISR(USART_RXC_vect)
{
	static volatile uint8_t RxNum=0;
	
	if(RxNum == 0) cbIn[cbInEnd].channel = (uint8_t)UDR;
	if(RxNum == 1) cbIn[cbInEnd].control = (uint8_t)UDR;
	if(RxNum == 2) cbIn[cbInEnd].value   = (uint8_t)UDR;

	if(++RxNum >= 3) 
	{
		RxNum = 0;
		if(++cbInEnd == cbIn_MAX) cbInEnd = 0;
	}	
}


/*******************************************************************************
* Function Name  : USART_UDRE_vect
* Description    : UART Transfer interrupt routine
*******************************************************************************/
ISR(USART_UDRE_vect)
{
	static uint8_t posCnt=0;

    if(TxFlag)
    {
        if(posCnt >= 3) 
        {
            posCnt = 0;
            TxFlag = 0;
            UCSRB &= ~(1<<UDRIE);
        }
        else
        {
            UDR = midiMessage[posCnt++];
        }            
    }	
}
