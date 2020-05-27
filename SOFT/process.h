/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PROCESS_H
#define PROCESS_H


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported structures ------------------------------------------------------ */
/* Exported functions ------------------------------------------------------- */
void ProcessOutBuf(struct OutBufType *OutBuf, struct IOtype *IO);
void SendFaders(struct IOtype *IO);


#endif// PROCESS_H
