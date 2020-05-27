/* Includes ------------------------------------------------------------------*/
#include "indication.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private constaints --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t adcEnd = 4;

volatile uint16_t adcA, adcB, adcC, adcX;
volatile uint8_t cX = 0;


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : ResetIO
* Description    : Reset all input/output data
* Input          : pointer to IO structure
*******************************************************************************/
void ResetIO(struct IOtype *IO)
{
	for(uint8_t i=0; i<5; i++) 
		IO->buttons[i] = 0;

	for(uint8_t i=0; i<20; i++) 
		IO->light[i] = 0;

	for(uint8_t i=0; i<8; i++) 
		IO->RGB[i].R = IO->RGB[i].G = IO->RGB[i].B = 0;

	for(uint8_t i=0; i<7; i++) 
		IO->lightIN[i] = IO->lightFL[i] = 0;
}


/*******************************************************************************
* Function Name  : SetIO
* Description    : Set all input/output data
* Input          : pointer to IO structure
*******************************************************************************/
void SetIO(struct IOtype *IO)
{
	for(uint8_t i=0; i<20; i++) 
		IO->light[i] = 0xFF;
	
	for(uint8_t i=0; i<8; i++) 
		IO->RGB[i].R = IO->RGB[i].G = IO->RGB[i].B = 0xFF;

	for(uint8_t i=0; i<7; i++) 
		IO->lightIN[i] = IO->lightFL[i] = 0xFF;
}


/*******************************************************************************
* Function Name  : ProcessIO
* Description    : HAL: indication and buttons scan
* Input          : pointer to IO structure
*******************************************************************************/
void ProcessIO(struct IOtype *IO)
{

//8 quant operations
	static uint8_t number5 = 0, number8 = 0;

	if(adcEnd == 0)
	{
		if(++number8 >= 8) number8 = 0;

		//DC	
		BitWrite((number8 & 1),PORTC,1);
		BitWrite((number8 & 2),PORTC,5);
		BitWrite((number8 & 4),PORTC,7);

		//RGB backlight
		OCR1B = IO->RGB[number8].R;
		OCR2  = IO->RGB[number8].G;
		OCR1A = IO->RGB[number8].B;

		//ADC
		if(cX == 16)
		{ 
			IO->currFader[24] = adcX >> 4; 
			cX = 0; 
			adcX = 0;
		}
		//IO->currFader[24] = adcX;

		IO->currFader[number8] 	 = adcA;
		IO->currFader[number8+8] = adcB;
		IO->currFader[number8+16]= adcC;

		adcA = adcB = adcC = 0;

		adcEnd = 4;
		ADCSRA |= (1<<ADSC);
	}
//end 8


//5 quant operations

	//Shift registers
	BitReset(PORTB, 3);		//задвинем информацию со жмякалок в 165е
	asm("nop");
	asm("nop");
	BitSet(PORTB, 3);

	PORTC &= ~((1<<PC0)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<6));
	IO->buttons[number5] = 0;

	SPDR = ~(IO->light[number5+10]); 
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется "787878787" ******** ******** ********
	IO->buttons[number5] |= SPDR; 
	
	SPDR = (IO->light[number5+15]); //7seg [15-18]
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** "787878787" ******** ********
	uint16_t A = SPDR << 8;
	IO->buttons[number5] |= A;	

	SPDR = ~(IO->light[number5+5]); //bar
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** ******** "787878787" ********

	SPDR = ~(IO->light[number5]);   //beginning
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** ******** ******** "787878787"

	if(++number5 >= 5) number5 = 0;	
		
//_delay_us(200);

	BitSet(PORTB, 4);		//задвинем иллюминацию в 595е
	asm("nop");
	asm("nop");
	BitReset(PORTB, 4);
	
	if(number5 == 0) BitSet(PORTC, 0); //дешифрация одной из 5 линий
	if(number5 == 1) BitSet(PORTC, 2);
	if(number5 == 2) BitSet(PORTC, 3);
	if(number5 == 3) BitSet(PORTC, 4);
	if(number5 == 4) BitSet(PORTC, 6);	
//end 5

}


/*******************************************************************************
* Function Name  : ADC_vect
* Description    : ADC Interrupt Handler
*******************************************************************************/
ISR(ADC_vect) 
{	
	if(adcEnd == 0)
		return;
	else
	{	
		switch(adcEnd)
		{
		/*	case 4: adcX = ADCL; adcX |= (uint8_t)(ADCH<<8); ADMUX = 5; break; //10bit
			case 3: adcA = ADCL; adcA |= (uint8_t)(ADCH<<8); ADMUX = 3; break;
			case 2: adcB = ADCL; adcB |= (uint8_t)(ADCH<<8); ADMUX = 1; break;
			case 1: adcC = ADCL; adcC |= (uint8_t)(ADCH<<8); ADMUX = 0; break;*/
			case 4: adcX += ADCH >> 1;  ADMUX = 5 | 0b01100000; cX++; break; //8bit
			case 3: adcA = ADCH >> 1;   ADMUX = 1 | 0b01100000; break;
			case 2: adcC = ADCH >> 1;   ADMUX = 3 | 0b01100000; break;
			case 1: adcB = ADCH >> 1;   ADMUX = 0 | 0b01100000; break;
		}

		adcEnd--;
		ADCSRA |= (1<<ADSC);
	}
}
