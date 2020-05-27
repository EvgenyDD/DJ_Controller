#ifndef INDICATION_H
#define INDICATION_H

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (1<<(m)))
#define bit_clear(p,m) ((p) &= ~(1<<(m)))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BitIsSet(reg, bit) ((reg & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) ((reg & (1<<(bit))) == 0)


/* Structures */
extern struct IOtype
{
	unsigned char light[20]; 	//��� ���������
	unsigned char lightIN[7];
	unsigned char lightFL[7];
	struct led_rgb				//RGB ���������� �� �����
	{
		unsigned char R, G, B;
	} RGB[8];

	unsigned int buttons[5]; 	//��� ������, ������ �� ����

	unsigned char currFader[25];	//��� ���������� ���������, ������� �� �����
	unsigned char lastFader[25];	//��� ���������� ���������, ������� �� �����

	unsigned int lastEnc;
	unsigned int currEnc;
	unsigned int lastButtons[5];

	unsigned char Flash, Blink;
	unsigned int fuck;
}IO; 


extern struct OutBufType
{
	unsigned char barLeft, barRight;
	unsigned char levelLeft, levelRight;
	unsigned char digit[4];
}OutBuf; 



/* DECLARATIONS */
void ResetIO(struct IOtype *IO);
void SetIO(struct IOtype *IO);
void ProcessIO(struct IOtype *IO);
unsigned int timeDiff(unsigned int, unsigned int);


#endif// INDICATION_H

