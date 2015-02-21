/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INDICATION_H
#define INDICATION_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (m))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)


/* Exported define -----------------------------------------------------------*/
#define MIDI_MAX	0x7F
#define MIDI_NULL	0x00


/* Exported structures ------------------------------------------------------ */
struct IOtype
{
	uint8_t light[20]; 		//вся индикация
	uint8_t lightIN[7];
	uint8_t lightFL[7];

	struct led_rgb			//RGB светодиоды на пэдах
	{
		uint8_t R, G, B;
	} RGB[8];

	uint16_t buttons[5]; 	//все кнопки, каждой по биту

	uint8_t lastFader[25], currFader[25];	//все переменные резисторы, каждому по байту

	uint16_t lastEnc, currEnc;
	uint16_t lastButtons[5];

	uint8_t Flash, Blink;
}; 


struct OutBufType
{
	uint8_t barLeft, barRight;
	uint8_t levelLeft, levelRight;
	uint8_t digit[4];
}; 


struct cbType
{
	uint8_t channel;
	uint8_t control;
	uint8_t value; 
};


/* Exported functions ------------------------------------------------------- */
void ResetIO(struct IOtype *IO);
void SetIO(struct IOtype *IO);
void ProcessIO(struct IOtype *IO);

void MidiAddMsg(uint8_t, uint8_t, uint8_t);


#endif// INDICATION_H
