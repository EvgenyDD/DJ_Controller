/*    ���������� ��� ������ � �������� �� ����������� HT1621 
      �� ��������� �������� �����
      
      � ��� - 12 ����, � ����� ����������� ��������
      � ������� ���� ��������� �����. 
*/

#include "HT1621.h"
#include <avr/io.h>
char digit[10]={ 0x7D, 0x60, 0x3E, 0x7A, 0x63, 0x5B, 0x5F, 0x70, 0x7F, 0x7B }; //������� 0-9

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
  SendBit_1621(0x80,4); //WRite flag code �100� and 9 bit command instruction 
  SendBit_1621(command,8); //no change clock output instruction, for convenience 
  CS1; //directly WRite command high bit as �0� 
}

void Write_1621(unsigned char addr,unsigned char data) 
{ 
  CS0; 
  addr <<=3;
  addr -= 4;
  SendBit_1621(0xa0,3); //WRite flag code �101� 
  SendBit_1621(addr,6); //WRite addr high 6 bits 
  SendDataBit_1621(data,8);  //WRite DATAa low 4 bits 
  CS1; 
} 

void Clear_1621() 
{ 
  CS0; 
  SendBit_1621(0xa0,3); //WRite flag code �101� 
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
  SendBit_1621(0xa0,3); //WRite flag code �101� 
  SendBit_1621(addr,6); //WRite addr high 6 bits 
  SendDataBit_1621(data,8);  //WRite DATAa low 4 bits 
  CS1; 
} 


void num3(uint8_t k, uint8_t n1, uint8_t n2, uint8_t n3)
{
	uint8_t m;
	m=k/100;
   	WriteDigit_1621(n1,m,0);
    m=k%100;
    m=m/10;
   	WriteDigit_1621(n2,m,0); 
    m=k%10;
   	WriteDigit_1621(n3,m,1);
}

void num5(unsigned int p, uint8_t n1, uint8_t n2, uint8_t n3,
uint8_t n4, uint8_t n5)
{
	unsigned int y;
	y=p/10000;
	WriteDigit_1621(n1,y,0);
	y=p%10000;
	y=y/1000;
	WriteDigit_1621(n2,y,0);
	y=p%1000;
	y=y/100;
	WriteDigit_1621(n3,y,0);
	y=p%100;
	y=y/10;
	WriteDigit_1621(n4,y,0); 
	y=p%10;
	WriteDigit_1621(n5,y,1);
}
