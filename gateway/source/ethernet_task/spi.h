/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*              实现spi2功能的头文件               */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __SPI2_H
#define __SPI2_H	 

void SPI_Configuration(void);
void SPI_GPIO_Configuration(void);
void SPI_WriteByte(unsigned char TxData);
unsigned char SPI_ReadByte(void);
void SPI_CrisEnter(void);
void SPI_CrisExit(void);
void SPI_CS_Select(void);
void SPI_CS_Deselect(void);
void W5500_Reset(void);
#endif 

