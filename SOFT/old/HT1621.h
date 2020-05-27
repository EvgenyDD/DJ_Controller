/*    EXAMPLE
Init:      
  SendCmd(BIAS);  //setup bias and working period 
  SendCmd(SYSEN); //start system oscillator 
  SendCmd(LCDON); //включаем экран
....
  SendCmd(LCDOFF); //выключаем экран
....
  Write_1621(5,0xFF); - запись в 5_цифру значения "FF"
                                          +++0b   1111 0000 -> HEX
                                          +++7seg HCBA DEGF
  WriteDigit_1621(1,7,0); - запись в 1_цифру "7"
  WriteDigit_1621(1,7,1); - запись в 1_цифру "7."
  Clear_1621(); - очищаем дисплей
...
  Write_1621(ЧЧ,0xFF); - ЧЧ={13,14,15,16} - символы над строкой
*/
#ifndef HT1621_H
#define HT1621_H

//#include <avr/io.h>

#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define bit_set(p,m) ((p) |= (1<<(m)))
#define bit_clear(p,m) ((p) &= ~(1<<(m)))

//PORTS
#define CS0 bit_clear(PORTA, 6)  
#define CS1 bit_set(PORTA, 6)
 
#define WR0 bit_clear(PORTA, 4) 
#define WR1 bit_set(PORTA, 4) 
   
#define DATA0 bit_clear(PORTA, 2) 
#define DATA1 bit_set(PORTA, 2)

//SYSTEM(chip) COMMANDS
#define BIAS 0x52                 
#define SYSEN 0x02              //system clock enable
#define LCDON 0x06    //turn on LCD
#define LCDOFF 0x04   //turn off LCD

#define TONE4K 0x80         //choose 4kHz tone(default)
#define TONE2K 0xC0         //choose 2kHz tone
#define TONEON 0x12   //BUZZER on
#define TONEOFF 0x10  //BUZZER off

void Write_1621(unsigned char addr,unsigned char data);
void WriteDigit_1621(unsigned char addr,unsigned char data,unsigned char dotPoint);
void Clear_1621();
void SendCmd(unsigned char command);

void SendBit_1621(unsigned char data, unsigned char cnt);
void SendDataBit_1621(unsigned char data,unsigned char cnt); 
#endif

/* program example(main.c)

#include "HT1621.h"
#define FREQ 20000000UL
#define delay_ms(ms) __delay_cycles((FREQ/1000)*(ms))
/////////////////////////////////////////////////////////////////////////////////
void main() 
{ 
  DDRA = (1 << PB0) | (1 << PB2) | (1 << PB4); 	
  PORTA = (0 << PB0) | (0 << PB2) | (0 << PB4);

  SendCmd(BIAS); //setup bias and working period 
  SendCmd(SYSEN); //start system oscillator 
  SendCmd(LCDON); //switch on LCD bias generator 
  SendCmd(TONE2K);

do{  
  for(int p=0; p<1000; p++){
    int y=p;
    y=p/100;
   WriteDigit_1621(1,y,1);
    y=p%100;
    y=y/10;
   WriteDigit_1621(2,y,0); 
    y=p%10;
   WriteDigit_1621(3,y,1); 
   WriteDigit_1621(4,0,0);
    delay_ms(80);
      if(p%10==0){ SendCmd(TONEON); delay_ms(1);}
      else SendCmd(TONEOFF);
   }

  Clear_1621();
  PORTC_Bit7=1; 
  delay_ms(700);
  SendCmd(TONEON);
  PORTC_Bit7=0;
  delay_ms(50);
  SendCmd(TONEOFF);
}while(1);
}

*/

