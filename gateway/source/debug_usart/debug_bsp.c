/**
*********************************************************************************************************
*                                        		gateway
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.01
* 
*********************************************************************************************************
**/

/*
*********************************************************************************************************
Includes 
*********************************************************************************************************
*/
#include "hc32_ddl.h"
#include "debug_bsp.h"
#include <stdarg.h>
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
/* USART channel definition */
#define DEBUG_CH                        (M4_USART1)

/* USART baudrate definition */
#define DEBUG_BAUDRATE                  (512000)

/* USART RX Port/Pin definition */
#define DEBUG_RX_PORT                   (PortB)
#define DEBUG_RX_PIN                    (Pin01)
#define DEBUG_RX_FUNC                   (Func_Usart1_Rx)

/* USART TX Port/Pin definition */
#define DEBUG_TX_PORT                   (PortB)
#define DEBUG_TX_PIN                    (Pin01)
#define DEBUG_TX_FUNC                   (Func_Usart1_Tx)



 //定义串口1 发送缓冲区大小 256字节
#define DEBUG_TXBUFF_SIZE   256   
__align(8) char debug_buff[DEBUG_TXBUFF_SIZE]; 
/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void UsartRxIrqCallback(void);


/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void debug_usart_init(void)
{
    en_result_t enRet = Ok;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        UsartClkDiv_1,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSamleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };
    /* Enable peripheral clock */
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

    /* Initialize USART IO */
    PORT_SetFunc(DEBUG_TX_PORT, DEBUG_TX_PIN, DEBUG_TX_FUNC, Disable);

    /* Initialize UART */
    enRet = USART_UART_Init(DEBUG_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    /* Set baudrate */
    enRet = USART_SetBaudrate(DEBUG_CH, DEBUG_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
	
	USART_FuncCmd(DEBUG_CH, UsartTxAndTxEmptyInt, Enable);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
#define DEBUG_ENABLE 1
void debug_output(uint8_t* buff,uint8_t length)
{
#if (1 == DEBUG_ENABLE)
	for(uint8_t i = 0; i < length; i++)
	{
		while (Reset == USART_GetStatus(DEBUG_CH, UsartTxEmpty));
		USART_SendData(DEBUG_CH, buff[i]);
		while (Reset == USART_GetStatus(DEBUG_CH, UsartTxComplete)); 
		USART_ClearStatus(DEBUG_CH, UsartTxComplete);
	}
#endif	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void debug_printf(char* fmt,...) 
{  
	unsigned int length;
	/*可变参数输入*/
	va_list ap;
	va_start(ap,fmt);
	vsprintf(debug_buff,fmt,ap);
	va_end(ap);	
	/*可变参数输入*/
	length=strlen((const char*)debug_buff);
	debug_output((unsigned char*)debug_buff,length);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:将一个十六进制字节串转换成ASCII码表示的十六进制字符串 
*********************************************************************************************************
**/
void hex_to_ascii(unsigned char *pHex, unsigned char *pAscii, int nLen)
{
 unsigned char Nibble[2];
    unsigned int i,j;
    for (i = 0; i < nLen; i++)
	{
        Nibble[0] = (pHex[i] & 0xF0) >> 4;
        Nibble[1] = pHex[i] & 0x0F;
        for (j = 0; j < 2; j++)
		{
            if (Nibble[j] < 10)
			{    
                Nibble[j] += 0x30;
            }
            else
			{
                if (Nibble[j] < 16)
                    Nibble[j] = Nibble[j] - 10 + 'A';
            }
            *pAscii++ = Nibble[j];
        }      
    }      
}
/***********************************************END*****************************************************/

