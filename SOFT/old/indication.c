#include "indication.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

static unsigned char adcEnd = 4;

volatile unsigned int adcA, adcB, adcC, adcX;
volatile unsigned char cX = 0;





/* Reset all input/output data */
void ResetIO(struct IOtype *IO)
{

	for(unsigned char i=0; i<5; i++) 
	{
		IO->buttons[i] = 0;
	}

	for(unsigned char i=0; i<20; i++) 
	{
		IO->light[i] = 0;
	}

	for(unsigned char i=0; i<8; i++) 
	{
		IO->RGB[i].R = 0;
		IO->RGB[i].G = 0;
		IO->RGB[i].B = 0;
	}

	for(unsigned char i=0; i<7; i++) 
	{
		IO->lightIN[i] = 0;
		IO->lightFL[i] = 0;
	}

}

/* Reset all input/output data */
void SetIO(struct IOtype *IO)
{

	for(unsigned char i=0; i<20; i++) 
	{
		IO->light[i] = 0xFF;
	}

	for(unsigned char i=0; i<8; i++) 
	{
		IO->RGB[i].R = 0xFF;
		IO->RGB[i].G = 0xFF;
		IO->RGB[i].B = 0xFF;
	}

	for(unsigned char i=0; i<7; i++) 
	{
		IO->lightIN[i] = 0xFF;
		IO->lightFL[i] = 0xFF;
	}

}



/* Indication and buttons scan */
void ProcessIO(struct IOtype *IO)
{

//8 quant operations
	static unsigned char number5 = 0, number8 = 0;

	if(adcEnd == 0)
	{
		if(number8++ == 7) number8 = 0;

		//DC	
		switch(number8)
		{
			case 0: bit_clear(PORTC, 1); bit_clear(PORTC, 5); bit_clear(PORTC, 7); break;
			case 1: bit_set(PORTC, 1);   bit_clear(PORTC, 5); bit_clear(PORTC, 7); break;
			case 2: bit_clear(PORTC, 1); bit_set(PORTC, 5);   bit_clear(PORTC, 7); break;
			case 3: bit_set(PORTC, 1);   bit_set(PORTC, 5);   bit_clear(PORTC, 7); break;
			case 4: bit_clear(PORTC, 1); bit_clear(PORTC, 5); bit_set(PORTC, 7); break;
			case 5: bit_set(PORTC, 1);   bit_clear(PORTC, 5); bit_set(PORTC, 7); break;
			case 6: bit_clear(PORTC, 1); bit_set(PORTC, 5);   bit_set(PORTC, 7); break;
			case 7: bit_set(PORTC, 1);   bit_set(PORTC, 5);   bit_set(PORTC, 7); break;
		}

		//RGB backlight
		OCR1B = IO->RGB[number8].R;
		OCR2  = IO->RGB[number8].G;
		OCR1A = IO->RGB[number8].B;

		//ADC
		if(cX == 64){ IO->currFader[24] = adcX >> 6; cX = 0; adcX = 0;}
		//IO->currFader[24] = adcX;

		IO->currFader[number8] = adcA;
		IO->currFader[number8+8] = adcB;
		IO->currFader[number8+16] = adcC;

		adcA = 0;
		adcB = 0;
		adcC = 0;

		adcEnd = 4;
		// //run ADC ?
	}
//end 8


//5 quant operations

	//Shift registers
	bit_clear(PORTD, 0);		//задвинем информацию со жмякалок в 165е
	asm("nop");
	asm("nop");
	bit_set(PORTD, 0);

	PORTC &= ~((1<<PC0)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<6));
	IO->buttons[number5] = 0;

	SPDR = ~(IO->light[number5+10]); 
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется "787878787" ******** ******** ********
	(IO->buttons[number5]) |= SPDR; 
	
	SPDR = (IO->light[number5+15]); //7seg [15-18]
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** "787878787" ******** ********
	unsigned int A = SPDR << 8;
	(IO->buttons[number5]) |= A;	

	SPDR = ~(IO->light[number5+5]); //bar
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** ******** "787878787" ********

	SPDR = ~(IO->light[number5]);   //beginning
		while( !( SPSR & (1<<SPIF) ) ); // ждем пока перекинется ******** ******** ******** "787878787"

	number5++;
	if(number5 == 5) number5 = 0;	
		
//_delay_us(200);

	bit_set(PORTB, 4);		//задвинем иллюминацию в 595е
	asm("nop");
	asm("nop");
	bit_clear(PORTB, 4);
	
	if(number5 == 0) bit_set(PORTC, 0); //дешифрация одной из 5 линий
	if(number5 == 1) bit_set(PORTC, 2);
	if(number5 == 2) bit_set(PORTC, 3);
	if(number5 == 3) bit_set(PORTC, 4);
	if(number5 == 4) bit_set(PORTC, 6);
	
	
	ADCSRA |= (1<<ADSC);

//end 5

}




// ADC Interrupt 
ISR(ADC_vect) 
{	

	sei();

	if(adcEnd == 0)
	{
		return;
	}
	else
	{	
		switch(adcEnd)
		{
		
		/*	case 4: adcX = ADCL; adcX |= (unsigned char)(ADCH<<8); ADMUX = 5; break; //10bit
			case 3: adcA = ADCL; adcA |= (unsigned char)(ADCH<<8); ADMUX = 3; break;
			case 2: adcB = ADCL; adcB |= (unsigned char)(ADCH<<8); ADMUX = 1; break;
			case 1: adcC = ADCL; adcC |= (unsigned char)(ADCH<<8); ADMUX = 0; break;*/
			case 4: adcX += ADCH >> 1;  ADMUX = 5 | 0b01100000; cX++; break; //8bit
			case 3: adcA = ADCH >> 1;   ADMUX = 1 | 0b01100000; break;
			case 2: adcC = ADCH >> 1;   ADMUX = 3 | 0b01100000; break;
			case 1: adcB = ADCH >> 1;   ADMUX = 0 | 0b01100000; break;
		}

		adcEnd--;
		ADCSRA |= (1<<ADSC);
	}
}


