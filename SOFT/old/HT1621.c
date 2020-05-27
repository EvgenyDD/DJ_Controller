/*    Ѕиблиотека дл€ работы с дисплеем на контроллере HT1621 
      от кассового аппарата ќ–»ќЌ
      
      ¬ ∆ » - 12 цифр, и много надстрочных символов
      ¬ дисплее есть генератор звука. 
*/

#include "HT1621.h"
#include <avr/io.h>
char digit[10]={ 0x7D, 0x60, 0x3E, 0x7A, 0x63, 0x5B, 0x5F, 0x70, 0x7F, 0x7B }; //циферки 0-9

void SendBit_1621(unsigned char data, unsigned char cnt)  //DATAa high cnt low WRite to HT1621,high bit first 
{ 
  unsigned char i; 
  for(i=0; i<cnt; i++){ 
    if((data & 0x80)==0) DATA0;
    else DATA1; 
    WR0; 
    data <<= 1; 
    asm("nop"); 
    WR1;   
  } 
}

void SendDataBit_1621(unsigned char data,unsigned char cnt)  //DATAa low cnt bit WRite to the HT1621, low bit first
{ 
  unsigned char i; 
  for(i =0; i < cnt; i++) { 
    if((data&0x01)==0) DATA0; 
    else DATA1; 
    WR0; 
    data >>= 1;
    asm("nop"); 
    WR1; 
     
  } 
}

void SendCmd(unsigned char command) 
{ 
  CS0; 
  SendBit_1621(0x80,4); //WRite flag code Ф100Ф and 9 bit command instruction 
  SendBit_1621(command,8); //no change clock output instruction, for convenience 
  CS1; //directly WRite command high bit as Ф0Ф 
}

void Write_1621(unsigned char addr,unsigned char data) 
{ 
  CS0; 
  addr <<=3;
  addr -= 4;
  SendBit_1621(0xa0,3); //WRite flag code Ф101Ф 
  SendBit_1621(addr,6); //WRite addr high 6 bits 
  SendDataBit_1621(data,8);  //WRite DATAa low 4 bits 
  CS1; 
} 

void Clear_1621() 
{ 
  CS0; 
  SendBit_1621(0xa0,3); //WRite flag code Ф101Ф 
  for(int u=0; u<17; u++){
    Write_1621(u,0); 
  }
  CS1;
}

void WriteDigit_1621(unsigned char addr,unsigned char data,unsigned char dotPoint) 
{ 
  CS0; 
  addr <<=3;
  addr -= 4;
  data = digit[data];
  if(dotPoint) data |= 0x80;
  SendBit_1621(0xa0,3); //WRite flag code Ф101Ф 
  SendBit_1621(addr,6); //WRite addr high 6 bits 
  SendDataBit_1621(data,8);  //WRite DATAa low 4 bits 
  CS1; 
} 
