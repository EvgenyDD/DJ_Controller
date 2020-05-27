/* Name: main.c
 * Project: MIDI DJ CJ CONTROLLER 3.0 USB + LCD
 * Author: Avarges - avargesnano.narod.ru
 * Creation Date: 04-02-2010
 * Modified: 14-10-2011
 * Copyright: (c) 2010-2011 by Avarges
 * License: GPL.
 *
 * MIDI interface based on
 * Project: V-USB MIDI device on Low-Speed USB
 * Author: Martin Homuth-Rosemann
 * Creation Date: 2008-03-11
 * Copyright: (c) 2008 by Martin Homuth-Rosemann.
 * License: GPL.
 *
 */

#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbdrv.h"
#include "lcd_lib.h"

// Если дисплей не подключен - компилировать со значением 0
#define LCD_CONNECTED     1
//Анодом подключена подсветка или катодом к МК (катод = 0)
#define LED_ANOD	1

#if DEBUG_LEVEL > 0
#	warning "Never compile production devices with debugging enabled"
#endif

// This descriptor is based on http://www.usb.org/developers/devclass_docs/midi10.pdf
// 
// Appendix B. Example: Simple MIDI Adapter (Informative)
// B.1 Device Descriptor
//
static PROGMEM char deviceDescrMIDI[] = {	/* USB device descriptor */
	18,			/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,	/* descriptor type */
	0x10, 0x01,		/* USB version supported */
	0,			/* device class: defined at interface level */
	0,			/* subclass */
	0,			/* protocol */
	8,			/* max packet size */
	USB_CFG_VENDOR_ID,	/* 2 bytes */
	USB_CFG_DEVICE_ID,	/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	1,			/* manufacturer string index */
	2,			/* product string index */
	0,			/* serial number string index */
	1,			/* number of configurations */
};

// B.2 Configuration Descriptor
static PROGMEM char configDescrMIDI[] = {	/* USB configuration descriptor */
	9,			/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,	/* descriptor type */
	101, 0,			/* total length of data returned (including inlined descriptors) */
	2,			/* number of interfaces in this configuration */
	1,			/* index of this configuration */
	0,			/* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
	USBATTR_SELFPOWER,	/* attributes */
#else
	USBATTR_BUSPOWER,	/* attributes */
#endif
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */

// B.3 AudioControl Interface Descriptors
// The AudioControl interface describes the device structure (audio function topology) 
// and is used to manipulate the Audio Controls. This device has no audio function 
// incorporated. However, the AudioControl interface is mandatory and therefore both 
// the standard AC interface descriptor and the classspecific AC interface descriptor 
// must be present. The class-specific AC interface descriptor only contains the header 
// descriptor.

// B.3.1 Standard AC Interface Descriptor
// The AudioControl interface has no dedicated endpoints associated with it. It uses the 
// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl 
// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
	/* AC interface descriptor follows inline: */
	9,			/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	0,			/* index of this interface */
	0,			/* alternate setting for this interface */
	0,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* */
	1,			/* */
	0,			/* */
	0,			/* string index for interface */

// B.3.2 Class-specific AC Interface Descriptor
// The Class-specific AC interface descriptor is always headed by a Header descriptor 
// that contains general information about the AudioControl interface. It contains all 
// the pointers needed to describe the Audio Interface Collection, associated with the 
// described audio function. Only the Header descriptor is present in this device 
// because it does not contain any audio functionality as such.
	/* AC Class-Specific descriptor */
	9,			/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	9, 0,			/* wTotalLength */
	1,			/* */
	1,			/* */

// B.4 MIDIStreaming Interface Descriptors

// B.4.1 Standard MS Interface Descriptor
	/* interface descriptor follows inline: */
	9,			/* length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	1,			/* index of this interface */
	0,			/* alternate setting for this interface */
	2,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* AUDIO */
	3,			/* MS */
	0,			/* unused */
	0,			/* string index for interface */

// B.4.2 Class-specific MS Interface Descriptor
	/* MS Class-Specific descriptor */
	7,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	65, 0,			/* wTotalLength */

// B.4.3 MIDI IN Jack Descriptor
	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	1,			/* EMBEDDED bJackType */
	1,			/* bJackID */
	0,			/* iJack */

	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	2,			/* EXTERNAL bJackType */
	2,			/* bJackID */
	0,			/* iJack */

//B.4.4 MIDI OUT Jack Descriptor
	9,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	3,			/* MIDI_OUT_JACK descriptor */
	1,			/* EMBEDDED bJackType */
	3,			/* bJackID */
	1,			/* No of input pins */
	2,			/* BaSourceID */
	1,			/* BaSourcePin */
	0,			/* iJack */

	9,			/* bLength of descriptor in bytes */
	36,			/* bDescriptorType */
	3,			/* MIDI_OUT_JACK bDescriptorSubtype */
	2,			/* EXTERNAL bJackType */
	4,			/* bJackID */
	1,			/* bNrInputPins */
	1,			/* baSourceID (0) */
	1,			/* baSourcePin (0) */
	0,			/* iJack */


// B.5 Bulk OUT Endpoint Descriptors

//B.5.1 Standard Bulk OUT Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x1,			/* bEndpointAddress OUT endpoint number 1 */
	3,			/* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack  */
	1,			/* baAssocJackID (0) */


//B.6 Bulk IN Endpoint Descriptors

//B.6.1 Standard Bulk IN Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x81,			/* bEndpointAddress IN endpoint number 1 */
	3,			/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack (0) */
	3,			/* baAssocJackID (0) */
};

uchar usbFunctionDescriptor(usbRequest_t * rq)
{

	if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
		usbMsgPtr = (uchar *) deviceDescrMIDI;
		return sizeof(deviceDescrMIDI);
	} else {		/* must be config descriptor */
		usbMsgPtr = (uchar *) configDescrMIDI;
		return sizeof(configDescrMIDI);
	}
}


static uchar sendEmptyFrame;
uchar iii;
uchar midiMsg[8];
uchar midiPst[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                     70,71,72,73,74,75,76,77,1,2,3,4,5,6,7,0}; // Default midi-preset 
                     // first 16 is chanels 0x00..0x0F
                     // second 16 is controls# 0x00..0x7F
unsigned char lastis77 = 0;
unsigned char confwrite = 0;

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {	/* class request type */

		/*  Prepare bulk-in endpoint to respond to early termination   */
		if ((rq->bmRequestType & USBRQ_DIR_MASK) ==
		    USBRQ_DIR_HOST_TO_DEVICE) 
		sendEmptyFrame = 1;

	}

	return 0xff;
}

uchar ReadEEPROM(unsigned char addr)
{
    while(EECR & (1 << EEWE));
    EEARL = addr;
    EECR |= 1 << EERE;
    return EEDR;
}

void ReadoutEEPROM(void)
{ 
	uchar re_1=0;
	uchar re_2=0;
	while (re_1 < 0x20)
		
	{ midiMsg[0] = 0x0E;
	  midiMsg[1] = 0xE0+re_2; re_2++;
	  midiMsg[2]=ReadEEPROM(re_1); re_1++;
	  midiMsg[3]=ReadEEPROM(re_1); re_1++;
	  midiMsg[4] = 0x0E;
	  midiMsg[5] = 0xE0+re_2; re_2++;
	  midiMsg[6]=ReadEEPROM(re_1); re_1++;
	  midiMsg[7]=ReadEEPROM(re_1); re_1++;
	  iii = 8;
	  
	  while ( !(usbInterruptIsReady()) ) usbPoll();
      usbSetInterrupt(midiMsg, iii);
      while ( !(usbInterruptIsReady()) ) usbPoll();
	  }
}

// Read MIDI preset from EEPROM (if uploaded later)
void PresetInit(void)
{
	uchar re_1=0;
	uchar re_2=0;
	while (re_1 < 0x20)
	{ 
	  re_2=ReadEEPROM(re_1); 
	  if (re_2 < 0x80) { midiPst[re_1]=re_2; }
	  re_1++;
    }	
}

void WriteEEPROM(unsigned char addr, unsigned char val)
{
	while(EECR & (1 << EEWE));
    EEARL = addr;
    EEDR = val;
    cli();
    EECR |= 1 << EEMWE;
    EECR |= 1 << EEWE;  /* must follow within a couple of cycles -- therefore cli() */
    sei();	
}

void usbFunctionWriteOut(uchar * data, uchar len)
{
unsigned char cmdbyte;
unsigned char valuebyte;
unsigned char confread = 0;

if (len>3) 
{ 
	data++;
	if (*data++ == 0xE1) { // received config-like message
	cmdbyte = *data++;
	valuebyte = *data;
	// cmdbyte: 0x77 - first step config active
	// 0x55: second step config active (i want WRITE config)
	// 0x66: i want READ config
	// 0x7E: deactive config
	// 

	//
    if ((cmdbyte==0x55) && (lastis77 ==1)) {confwrite = 1;}
    if ((cmdbyte==0x66) && (lastis77 ==1)) {confread = 1; confwrite = 0;}
    if (cmdbyte==0x7E) {confwrite = 0; PresetInit(); }

    lastis77 = 0;
	if (cmdbyte==0x77) {lastis77 = 1; confwrite = 0;} else {lastis77 = 0;}
	
	if (confwrite==1) { WriteEEPROM((cmdbyte & 0x1F),(valuebyte & 0x7F)); } // cmdbyte = addr, value is value	
	
	if (confread==1) { confread=0; ReadoutEEPROM(); }	
	}
}
	

}

/*---------------------------------------------------------------------------*/
/* hardwareInit                                                              */
/*---------------------------------------------------------------------------*/

static void hardwareInit(void)
{
	uchar i, j;

	/* activate pull-ups except on USB lines */
	USB_CFG_IOPORT =
	    (uchar) ~ ((1 << USB_CFG_DMINUS_BIT) |
		       (1 << USB_CFG_DPLUS_BIT));
	/* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT	/* use usbDeviceConnect()/usbDeviceDisconnect() if available */
	USBDDR = 0;		/* we do RESET by deactivating pullup */
	usbDeviceDisconnect();
#else
	USBDDR = 0xFF;//(1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT);
#endif

	j = 0;
	while (--j) {		/* USB Reset by device only required on Watchdog Reset */
		i = 0;
		while (--i);	/* delay >10ms for USB reset */
	}
#ifdef USB_CFG_PULLUP_IOPORT
	usbDeviceConnect();
#else
	USBDDR = (uchar) ~ ((1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT));	//= 0;	/*  remove USB reset condition */
#endif

// ADC Setup
// enable, prescaler = 2^7 (-> 12Mhz / 128 = 90 kHz)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	PORTB = 0b11100000;
	DDRB  = 0b00011111;

	PORTC = 0x00;
	DDRC = 0b01000000;

} // hardwareInit


int adc(uchar channel)
{
	// single ended channel 0..7
	ADMUX = channel & 0x07;
	// AREF ext., adc right adjust result
	ADMUX |= (0 << REFS1) | (0 << REFS0) | (0 << ADLAR);
	// adc start conversion
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC)) {
		;		// idle
	}
	return ADC;
}

// Load 8 special symbols into LCD RAM
void LCDdef8(void){
	uint8_t a, pcc, k;
	uint16_t i, j;
k=0x80; a=0b01000000;
 for (j=0; j<8; j++) {
	for (i=0; i<8; i++){
		if ( ((k >> i) & 1)== 1) { pcc = 31; } else { pcc = 0; }
		LCDsendCommand(a++);
		LCDsendChar(pcc);
		}
	k = (k >> 1)|0x80;
 }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

/* reverse:  переворачиваем строку s на месте */
void reverse(char s[])
{
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}

void myitoa(int n, char s[])
{
     int i;
 
     if (n < 0)  /* записываем знак */
         n = -n;          /* делаем n положительным числом */
     i = 0;
     do {       /* генерируем цифры в обратном порядке */
         s[i++] = n % 10 + '0';   /* берем следующую цифру */
     } while ((n /= 10) > 0);     /* удаляем */

     s[i] = '\0';
     reverse(s);
}

int main(void)
{
	int adcOld[13] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	uchar key, lastKey, En, En2, lastEn, freeslot;
	uchar channel = 0;
	uchar mux_n = 0;
	int value, lci, canter;
	uchar ledon, ledcnt = 0;
	char buffer[16];
	char *str;

	wdt_enable(WDTO_1S);
	hardwareInit();

	usbInit();
	
	PresetInit();

	sendEmptyFrame = 0;

	sei();

	channel = 0; 
	ledon = 0;
	key = 0xFF;
	lastKey = 0xFF;
	lastEn = 3;
	
#if LCD_CONNECTED > 0
	// Инициализация LCD
	LCDinit();
	
	// Загружаем собственный символ
	LCDdef8();

	// Выключаем курсор
	LCDcursorOFF();
	
	// Очищаем дисплей
	LCDclr(); 
#endif
	
	for (;;) {		/* main event loop */
		wdt_reset();
		usbPoll();
		
		En = PIND & 0b00000011;
		_delay_ms(2); 
		En2 = PIND & 0b00000011;
		
		// Выключается подсветка LCD если вышло время
		if (canter == 0) { 

			#if LED_ANOD > 0
			PORTC &= ~0b01000000;
			#else
			PORTC |= 0b01000000; 
			#endif
				    
			canter--; 
			} else { if (canter > 0) canter--; }

		iii = 0;
		if (usbInterruptIsReady()) { iii = 2; }

		if (iii==2) {
			freeslot = 2;
			iii = 0;
		
//////////// ENCODER 	
		if (En2 == En) { // it means - no contact bounce error
		 if (En != lastEn) {
		  if (lastEn == 3) { 
		  	  		if (En == 2) { // энкодер влево
					midiMsg[iii++] = 0x0b;
					midiMsg[iii++] = 0xb0+(midiPst[13] & 0x0F); // chanel
					midiMsg[iii++] = midiPst[13+0x10]; // control#
					midiMsg[iii++] = 0x00;
					freeslot--;
				}
					if (En == 1) { // энкодер вправо
					midiMsg[iii++] = 0x0b;
					midiMsg[iii++] = 0xb0+(midiPst[13] & 0x0F); // chanel
					midiMsg[iii++] = midiPst[13+0x10]; // control#
					midiMsg[iii++] = 0x7F;
					freeslot--;
				}
				
					
#if LCD_CONNECTED > 0
                    // Переходим в позицию
	                LCDGotoXY(12,0);
					//Формируем строку в буфере
	                myitoa(En,buffer);
                    //Выводим из буфера
	                LCDstring(buffer,1);
#endif

		  }
		  lastEn = En;
		 }
		}
		// END ENCODER
		
/////////////// PB5 ENCODER KEY
		if (freeslot > 0) {
		key = PINB & 0b00100000;
		if (key > 0) { En = 0x00; } else { En = 0x7F; }
		if (key != lastKey)
			{
				midiMsg[iii++] = 0x0b;
				midiMsg[iii++] = 0xb0+(midiPst[14] & 0x0F); // chanel
				midiMsg[iii++] = midiPst[14+0x10]; // control#
				midiMsg[iii++] = En;
				freeslot--;
				lastKey = key;
				
#if LCD_CONNECTED > 0
					// Размер свободной памяти (до кучи)
					LCDGotoXY(0,0);
					myitoa(freeRam (), buffer);
					LCDstring(buffer,strlen(buffer));
					// Значение кнопки энкодера
	                LCDGotoXY(14,0);
					if (En == 0) { str = "00"; } else { str = "7F"; }
	                LCDstring(str,2);
#endif
			}
		}

///////////  ADC
		if (freeslot > 0) { //adc check
		        // check analog input
		    if (channel == 0) { 
				if (7 == mux_n) { mux_n = 0; } else { mux_n++; }
				// Выставление адресов на мультиплексор
				PORTB &= 0b11100011;
		    	PORTB |= mux_n << 2; // change mux input line PB4..PB2
		    	_delay_ms(1);
		    	}
		    	
				value = adc(channel);	// 0..1023
				// hysteresis
				if (adcOld[channel+mux_n] - value > 7
					|| adcOld[channel+mux_n] - value < -7) {

					adcOld[channel+mux_n] = value;
					
					// MIDI CC msg
					midiMsg[iii++] = 0x0b;
					midiMsg[iii++] = 0xb0+(midiPst[channel+mux_n] & 0x0F); // chanel
					midiMsg[iii++] = midiPst[channel+mux_n+0x10]; // control#
					// Инвертирование значения кнопок
if ( (channel == 0) && (mux_n > 4) && (mux_n <= 7) ) { En = (~value >> 3) & 0x7F; } else { En = value >> 3; }
					midiMsg[iii++] = En;
					

                    
                #if LCD_CONNECTED > 0
					
					// Формируется строка вида CC# X [VAL] X-номер контролла, VAL-значение
					str = "CC# ";
					LCDGotoXY(0,0);
					LCDstring(str,4);
					
					lci = channel+mux_n+0x30;
					if (lci > 0x39) lci += 7;
					LCDGotoXY(4,0);
	                LCDsendChar(lci);
	                
	                str = " [";
					LCDGotoXY(5,0);
					LCDstring(str,2);
	            
	                LCDGotoXY(7,0);
	                myitoa(En, buffer);
	                LCDstring(buffer,3);
	                
	                lci = strlen(buffer);
	                
	                str = "]  ";
					LCDGotoXY(lci+7,0);
					LCDstring(str,4-lci);
	                
	                // Спецсимволы во вторую строку
	                lci = En >> 4;
	                LCDGotoXY(channel+mux_n,1);
	                LCDsendChar(lci);
				
				#endif
                    

				}
				if (channel == 0) { 
				if (7 == mux_n) { channel++; }
				} else { channel++; }

				if (6 == channel)
					channel = 0;
				} // endof adc check
				if (iii > 0) { 
				    // Включается подсветка LCD
				    #if LED_ANOD > 0
				    PORTC |= 0b01000000;
				    #else
				    PORTC &= ~0b01000000; 
				    #endif
				    // на ~3 секунды
					canter = 900;
				 	// Отправляется МИДИ-пакет
				 	usbSetInterrupt(midiMsg, iii);
				}
		}		// usbInterruptIsReady()
	}
	return 0;
}
