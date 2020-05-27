#ifndef USB_H
#define USB_H


extern volatile struct cbType
{
	unsigned char com;
	unsigned char channel;
	unsigned char control;
	unsigned char value; 
}cb[16];


void USBSendQueue(void);
void USBAddMsg(unsigned char com, unsigned char channel, unsigned char control, unsigned char value);
void USBAddMsg2(unsigned char com, unsigned char channel, unsigned char control, unsigned char value);

void UARTSendQueue(void);


#endif// USB_H
