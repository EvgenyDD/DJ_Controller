//*****************************************************************************
//
// File Name	: 'lcd_lib.c'
// Title		: 4 bit LCd interface
// Author		: Scienceprog.com - Copyright (C) 2007
// Modified	by	: Koryagin Andrey 2011
// Created		: 2007-06-18
// Modified		: 2011-10-12
// Version		: 1.1.1.MIDIDJCJ
// Target MCU	: Atmel AVR series
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
#include "lcd_lib.h"
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

void LCDsendChar(uint8_t ch)		//Sends Char to LCD
{
	LDP &= ~((0b11110000)>>(4-LCD_D4));
	LDP |=((ch&0b11110000)>>(4-LCD_D4));

	LCP|=1<<LCD_RS;
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);	
	LCP&=~(1<<LCD_RS);
	_delay_ms(1);
	
	LDP &= ~((0b11110000)>>(4-LCD_D4));
	LDP |=((ch&0b00001111)<<(LCD_D4));
	
	LCP|=1<<LCD_RS;
	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);	
	LCP&=~(1<<LCD_RS);
	_delay_ms(1);
}

void LCDsendCommand(uint8_t cmd)	//Sends Command to LCD
{
	LDP &= ~((0b11110000)>>(4-LCD_D4));
	LDP |=((cmd&0b11110000)>>(4-LCD_D4));

	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);

	LDP &= ~((0b11110000)>>(4-LCD_D4));
	LDP |=((cmd&0b00001111)<<(LCD_D4));

	LCP|=1<<LCD_E;		
	_delay_ms(1);
	LCP&=~(1<<LCD_E);
	_delay_ms(1);
}

void LCDinit(void)//Initializes LCD
{
	_delay_ms(1);
	LDP &= ~(1<<LCD_D7|1<<LCD_D6|1<<LCD_D5|1<<LCD_D4);
	//LCP &= ~(1<<LCD_E|1<<LCD_RW|1<<LCD_RS);
	LCP &= ~(1<<LCD_E|1<<LCD_RS);
	LDDR|=1<<LCD_D7|1<<LCD_D6|1<<LCD_D5|1<<LCD_D4;
	//LCDR|=1<<LCD_E|1<<LCD_RW|1<<LCD_RS;
	LCDR|=1<<LCD_E|1<<LCD_RS;
   //---------one------
	//4 bit mode
	LDP |= (1<<LCD_D5|1<<LCD_D4);
	LDP &= ~(1<<LCD_D7|1<<LCD_D6);
	LCP|= (1<<LCD_E);
	_delay_ms(1);
	LCP &= ~(1<<LCD_E);
	_delay_ms(1);
	//-----------two-----------
	LDP |= (1<<LCD_D5|1<<LCD_D4);
	LDP &= ~(1<<LCD_D7|1<<LCD_D6);
	LCP|= (1<<LCD_E);
	_delay_ms(1);
	LCP &= ~(1<<LCD_E);
	_delay_ms(1);
	//-------three-------------
	LDP |= (1<<LCD_D5);
	LDP &= ~(1<<LCD_D7|1<<LCD_D6|1<<LCD_D4);
	LCP|= (1<<LCD_E);
	_delay_ms(1);
	LCP &= ~(1<<LCD_E);
	_delay_ms(1);
	//--------4 bit--dual line---------------
	LCDsendCommand(0b00101000);
   //-----increment address, cursor shift------
	LCDsendCommand(0b00001110);
}

void LCDclr(void)				//Clears LCD
{
	LCDsendCommand(1<<LCD_CLR);
}

void LCDstring(uint8_t* data, uint8_t nBytes)	//Outputs string to LCD
{
	register uint8_t i;

	// check to make sure we have a good pointer
	if (!data) return;

	// print data
	for(i=0; i<nBytes; i++)
	{
		LCDsendChar(data[i]);
	}
}

void LCDGotoXY(uint8_t x, uint8_t y)	//Cursor to X Y position
{
	register uint8_t DDRAMAddr;
	// remap lines into proper order
	switch(y)
	{
	case 0: DDRAMAddr = LCD_LINE0_DDRAMADDR+x; break;
	case 1: DDRAMAddr = LCD_LINE1_DDRAMADDR+x; break;
	default: DDRAMAddr = LCD_LINE0_DDRAMADDR+x;
	}
	// set data address
	LCDsendCommand(1<<LCD_DDRAM | DDRAMAddr);
	
}

void LCDcursorOFF(void)	//turns OFF cursor
{
	LCDsendCommand(0x0C);
}
