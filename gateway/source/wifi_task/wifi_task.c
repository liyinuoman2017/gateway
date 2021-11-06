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
#include "hc32f46x_clk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_task.h"
#include "debug_bsp.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/




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
TaskHandle_t wifi_task_handler;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/

/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
/* USART channel definition */
#define WIFI_CH                        (M4_USART4)

/* USART baudrate definition */
#define WIFI_BAUDRATE                  (115200)

/* USART RX Port/Pin definition */
#define WIFI_RX_PORT                   (PortB)
#define WIFI_RX_PIN                    (Pin15)
#define WIFI_RX_FUNC                   (Func_Usart4_Rx)

/* USART TX Port/Pin definition */
#define WIFI_TX_PORT                   (PortB)
#define WIFI_TX_PIN                    (Pin14)
#define WIFI_TX_FUNC                   (Func_Usart4_Tx)

/* USART interrupt number  */
#define WIFI_RI_NUM                    (INT_USART4_RI)
#define WIFI_EI_NUM                    (INT_USART4_EI)
#define WIFI_TI_NUM                    (INT_USART4_TI)
#define WIFI_TCI_NUM                   (INT_USART4_TCI)


#define WIFI_RESET_PORT                   (PortA)
#define WIFI_RESET_PIN                    (Pin02)
#define RESET_IO_H    PORT_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN) 
#define RESET_IO_L    PORT_ResetBits(WIFI_RESET_PORT, WIFI_RESET_PIN) 

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
#include "rs458_analysis.h"
usart_frame_t   wifi_usart2_frame;
u8 wifi_judge_num=0;
void Usart4_RxIrqCallback(void);

/**
 *******************************************************************************
 ** \brief USART RX error irq callback function.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void Usart4_ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(WIFI_CH, UsartFrameErr))
    {
        USART_ClearStatus(WIFI_CH, UsartFrameErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(WIFI_CH, UsartParityErr))
    {
        USART_ClearStatus(WIFI_CH, UsartParityErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(WIFI_CH, UsartOverrunErr))
    {
        USART_ClearStatus(WIFI_CH, UsartOverrunErr);
    }
    else
    {
    }
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
void wifi_usart_init(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
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
    PORT_SetFunc(WIFI_RX_PORT, WIFI_RX_PIN, WIFI_RX_FUNC, Disable);
    PORT_SetFunc(WIFI_TX_PORT, WIFI_TX_PIN, WIFI_TX_FUNC, Disable);

    /* Initialize UART */
    enRet = USART_UART_Init(WIFI_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    /* Set baudrate */
    enRet = USART_SetBaudrate(WIFI_CH, WIFI_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int001_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4_RxIrqCallback;
    stcIrqRegiCfg.enIntSrc = WIFI_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = Int002_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4_ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = WIFI_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	
    /*Enable RX && RX interupt function*/
    USART_FuncCmd(WIFI_CH, UsartRx, Enable);
    USART_FuncCmd(WIFI_CH, UsartRxInt, Enable);
	
	USART_FuncCmd(WIFI_CH, UsartTxAndTxEmptyInt, Enable);


	//////RESET
    stc_port_init_t stcPortInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_Init(WIFI_RESET_PORT, WIFI_RESET_PIN, &stcPortInit);
	
	
}

extern char Connect_flag;  //外部变量声明，同服务器连接状态  0：还没有连接服务器  1：连接上服务器了

#define USART2_RXBUFF_SIZE   1024              //定义串口2 接收缓冲区大小 1024字节

extern char Usart2_RxCompleted ;               //外部声明，其他文件可以调用该变量
extern unsigned int Usart2_RxCounter;          //外部声明，其他文件可以调用该变量
extern char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //外部声明，其他文件可以调用该变量


char Usart2_RxCompleted = 0;            //定义一个变量 0：表示接收未完成 1：表示接收完成 
unsigned int Usart2_RxCounter = 0;      //定义一个变量，记录串口2总共接收了多少字节的数据
char Usart2_RxBuff[USART2_RXBUFF_SIZE]; //定义一个数组，用于保存串口2接收到的数据   

#define WiFi_printf       u2_printf           //串口2控制 WiFi
#define WiFi_RxCounter    Usart2_RxCounter    //串口2控制 WiFi
#define WiFi_RX_BUF       Usart2_RxBuff       //串口2控制 WiFi
#define WiFi_RXBUFF_SIZE  USART2_RXBUFF_SIZE  //串口2控制 WiFi



#define SSID   "tofan"                     //路由器SSID名称
#define PASS   "4006352166"                 //路由器密码

char *ServerIP = "47.106.234.88";           //存放服务器IP或是域名
int  ServerPort = 5188;                      //存放服务器的端口号区


void WiFi_ResetIO_Init(void);
char WiFi_SendCmd(char *cmd, int timeout);
char WiFi_Reset(int timeout);
char WiFi_JoinAP(int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_Smartconfig(int timeout);
char WiFi_WaitAP(int timeout);
char WiFi_GetIP(u16 timeout);
char WiFi_Get_LinkSta(void);
char WiFi_Get_Data(char *data, char *len, char *id);
char WiFi_SendData(char id, char *databuff, int data_len, int timeout);
char WiFi_Connect_Server(int timeout);
char WiFi_ConnectServer(void);
char wifi_mode = 0;     //联网模式 0：SSID和密码写在程序里   1：Smartconfig方式用APP发送
char Connect_flag;      //同服务器连接状态  0：还没有连接服务器  1：连接上服务器了


#include "debug_bsp.h"
#include <stdarg.h>
__align(8) char USART2_TxBuff[200];  

void u2_printf(char* fmt,...) 
{  
	unsigned int i,length;
	
	va_list ap;
	va_start(ap,fmt);
	vsprintf(USART2_TxBuff,fmt,ap);
	va_end(ap);	
	
	length=strlen((const char*)USART2_TxBuff);

	for(uint8_t i = 0; i < length; i++)
	{
		while (Reset == USART_GetStatus(WIFI_CH, UsartTxEmpty));
		USART_SendData(WIFI_CH, USART2_TxBuff[i]);
		while (Reset == USART_GetStatus(WIFI_CH, UsartTxComplete)); 
		USART_ClearStatus(WIFI_CH, UsartTxComplete);
	}		
}
#define DEBUG_TXBUFF_SIZE   256   
extern char debug_buff[DEBUG_TXBUFF_SIZE]; 
void u1_printf(char* fmt,...) 
{  
	unsigned int i,length;
	/*可变参数输入*/
	va_list ap;
	va_start(ap,fmt);
	vsprintf(debug_buff,fmt,ap);
	va_end(ap);	
	/*可变参数输入*/
	length=strlen((const char*)debug_buff);
	debug_output((unsigned char*)debug_buff,length);	
}
/*-------------------------------------------------*/
/*函数名：WiFi发送设置指令                         */
/*参  数：cmd：指令                                */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_SendCmd(char *cmd, int timeout)
{
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区 
	WiFi_printf("%s\r\n",cmd);                  //发送指令
	while(timeout--){                           //等待超时时间到0
		vTaskDelay(100);                          //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))            //如果接收到OK表示指令成功
			break;       						//主动跳出while循环
		debug_printf("%d ",timeout);               //串口输出现在的超时时间
	}
	debug_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else return 0;		         				//反之，表示正确，说明收到OK，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi复位                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Reset(int timeout)
{
	RESET_IO_L;                                    //复位IO拉低电平
	vTaskDelay(500);                                  //延时500ms
	RESET_IO_H;                                    //复位IO拉高电平	
	while(timeout--){                               //等待超时时间到0
		vTaskDelay(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"ready"))             //如果接收到ready表示复位成功
			break;       						    //主动跳出while循环
		debug_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	debug_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
	else return 0;		         				    //反之，表示正确，说明收到ready，通过break主动跳出while
}
/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_JoinAP(int timeout)
{		
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
	WiFi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,PASS); //发送指令	
	while(timeout--){                               //等待超时时间到0
		vTaskDelay(100);                             //延时1s
		//if(strstr(WiFi_RX_BUF,"WIFI GOT IP\r\n\r\nOK")) //如果接收到WIFI GOT IP表示成功
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP")) //如果接收到WIFI GOT IP表示成功
		{
			char delay=50;
			while(delay--)
			{ 
				vTaskDelay(100);
				if(strstr(WiFi_RX_BUF,"OK")) //如果接收到WIFI GOT IP表示成功
					return 0;
			}
				
		}			
			//break;       						    //主动跳出while循环
		debug_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	debug_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：WiFi_Smartconfig                         */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_Smartconfig(int timeout)
{	
	WiFi_RxCounter=0;                           //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);     //清空WiFi接收缓冲区     
	while(timeout--){                           //等待超时时间到0
		vTaskDelay(1000);                         //延时1s
		if(strstr(WiFi_RX_BUF,"connected"))     //如果串口接受到connected表示成功
			break;                              //跳出while循环  
		debug_printf("%d ",timeout);               //串口输出现在的超时时间  
	}	
	debug_printf("\r\n");                          //串口输出信息
	if(timeout<=0)return 1;                     //超时错误，返回1
	return 0;                                   //正确返回0
}
/*-------------------------------------------------*/
/*函数名：等待加入路由器                           */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_WaitAP(int timeout)
{		
	while(timeout--){                               //等待超时时间到0
		vTaskDelay(1000);                             //延时1s
		if(strstr(WiFi_RX_BUF,"WIFI GOT IP"))       //如果接收到WIFI GOT IP表示成功
			break;       						    //主动跳出while循环
		debug_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	debug_printf("\r\n");                              //串口输出信息
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1
	return 0;                                       //正确，返回0
}
/*-------------------------------------------------*/
/*函数名：等待连接wifi，获取IP地址                 */
/*参  数：ip：保存IP的数组                         */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ip_show_buff[20];
char WiFi_GetIP(u16 timeout)
{
	char *presult1,*presult2;
	char ip[50];
		                              	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零 
	
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区 
	vTaskDelay(1000);
	WiFi_printf("AT+CIFSR\r\n");                    //发送指令	
	while(timeout--){                               //等待超时时间到0
		vTaskDelay(100);                              //延时100ms
		if(strstr(WiFi_RX_BUF,"OK"))                //如果接收到OK表示成功
			break;       						    //主动跳出while循环
		debug_printf("%d ",timeout);                   //串口输出现在的超时时间
	}
	debug_printf("\r\n");                              //串口输出信息
	if(timeout<=0)
		return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到OK，返回1
	else
	{
		presult1 = strstr(WiFi_RX_BUF,"\"");
		if( presult1 != NULL ){
			presult2 = strstr(presult1+1,"\"");
			if( presult2 != NULL ){
				memcpy(ip,presult1+1,presult2-presult1-1);
				memcpy(ip_show_buff,ip,20);
				debug_printf("ESP8266的IP地址：%s\r\n",ip);     //串口显示IP地址
				return 0;    //正确返回0
			}else 
			return 2;  //未收到预期数据
		}else 
		return 3;      //未收到预期数据	
	}
}
/*-------------------------------------------------*/
/*函数名：获取连接状态                             */
/*参  数：无                                       */
/*返回值：连接状态                                 */
/*        0：无状态                                */
/*        1：有客户端接入                          */
/*        2：有客户端断开                          */
/*-------------------------------------------------*/
char WiFi_Get_LinkSta(void)
{
	char id_temp[10]={0};    //缓冲区，存放ID
	char sta_temp[10]={0};   //缓冲区，存放状态
	
	if(strstr(WiFi_RX_BUF,"CONNECT")){                 //如果接受到CONNECT表示有客户端连接	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		debug_printf("有客户端接入，ID=%s\r\n",id_temp);  //串口显示信息
		WiFi_RxCounter=0;                              //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);        //清空WiFi接收缓冲区     
		return 1;                                      //有客户端接入
	}else if(strstr(WiFi_RX_BUF,"CLOSED")){            //如果接受到CLOSED表示有链接断开	
		sscanf(WiFi_RX_BUF,"%[^,],%[^,]",id_temp,sta_temp);
		debug_printf("有客户端断开，ID=%s\r\n",id_temp);        //串口显示信息
		WiFi_RxCounter=0;                                    //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);              //清空WiFi接收缓冲区     
		return 2;                                            //有客户端断开
	}else return 0;                                          //无状态改变	
}
/*-------------------------------------------------*/
/*函数名：获取客户端数据                           */
/*        两组回车换行符\r\n\r\n作为数据的结束符   */
/*参  数：data：数据缓冲区                         */
/*参  数：len： 数据量                             */
/*参  数：id：  发来数据的客户端的连接ID           */
/*返回值：数据状态                                 */
/*        0：无数据                                */
/*        1：有数据                                */
/*-------------------------------------------------*/
char WiFi_Get_Data(char *data, char *len, char *id)
{
	char temp[10]={0};      //缓冲区
	char *presult;

	if(strstr(WiFi_RX_BUF,"\r\n\r\n")){                     //两个连着的回车换行作为数据的结束符
		sscanf(WiFi_RX_BUF,"%[^,],%[^,],%[^:]",temp,id,len);//截取各段数据，主要是id和数据长度	
		presult = strstr(WiFi_RX_BUF,":");                  //查找冒号。冒号后的是数据
		if( presult != NULL )                               //找到冒号
			sprintf((char *)data,"%s",(presult+1));         //冒号后的数据，复制到data
		WiFi_RxCounter=0;                                   //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);             //清空WiFi接收缓冲区    
		return 1;                                           //有数据到来
	} else return 0;                                        //无数据到来
}
/*-------------------------------------------------*/
/*函数名：服务器发送数据                           */
/*参  数：databuff：数据缓冲区<2048                */
/*参  数：data_len：数据长度                       */
/*参  数：id：      客户端的连接ID                 */
/*参  数：timeout： 超时时间（10ms的倍数）         */
/*返回值：错误值                                   */
/*        0：无错误                                */
/*        1：等待发送数据超时                      */
/*        2：连接断开了                            */
/*        3：发送数据超时                          */
/*-------------------------------------------------*/
char WiFi_SendData(char id, char *databuff, int data_len, int timeout)
{    
	int i;
	
	WiFi_RxCounter=0;                                 //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 
	WiFi_printf("AT+CIPSEND=%d,%d\r\n",id,data_len);  //发送指令	
    while(timeout--){                                 //等待超时与否	
		vTaskDelay(10);                                 //延时10ms
		if(strstr(WiFi_RX_BUF,">"))                   //如果接收到>表示成功
			break;       						      //主动跳出while循环
		debug_printf("%d ",timeout);                     //串口输出现在的超时时间
	}
	if(timeout<=0)return 1;                                   //超时错误，返回1
	else{                                                     //没超时，正确       	
		WiFi_RxCounter=0;                                     //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);               //清空WiFi接收缓冲区 	
		for(i=0;i<data_len;i++)WiFi_printf("%c",databuff[i]); //发送数据	
		while(timeout--){                                     //等待超时与否	
			vTaskDelay(10);                                     //延时10ms
			if(strstr(WiFi_RX_BUF,"SEND OK")){                //如果接受SEND OK，表示发送成功			 
			WiFi_RxCounter=0;                                 //WiFi接收数据量变量清零                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);           //清空WiFi接收缓冲区 			
				break;                                        //跳出while循环
			} 
			if(strstr(WiFi_RX_BUF,"link is not valid")){      //如果接受link is not valid，表示连接断开			
				WiFi_RxCounter=0;                             //WiFi接收数据量变量清零                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);       //清空WiFi接收缓冲区 			
				return 2;                                     //返回2
			}
	    }
		if(timeout<=0)return 3;      //超时错误，返回3
		else return 0;	            //正确，返回0
	}	
}
/*-------------------------------------------------*/
/*函数名：连接TCP服务器，并进入透传模式            */
/*参  数：timeout： 超时时间（100ms的倍数）        */
/*返回值：0：正确  其他：错误                      */
/*-------------------------------------------------*/
char WiFi_Connect_Server(int timeout)
{	
	WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	WiFi_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ServerIP,ServerPort);//发送连接服务器指令
	while(timeout--){                               //等待超时与否
		vTaskDelay(100);                              //延时100ms	
		if(strstr(WiFi_RX_BUF ,"CONNECT"))          //如果接受到CONNECT表示连接成功
			break;                                  //跳出while循环
		if(strstr(WiFi_RX_BUF ,"CLOSED"))           //如果接受到CLOSED表示服务器未开启
			return 1;                               //服务器未开启返回1
		if(strstr(WiFi_RX_BUF ,"ALREADY CONNECTED"))//如果接受到ALREADY CONNECTED已经建立连接
			return 2;                               //已经建立连接返回2
		debug_printf("%d ",timeout);                   //串口输出现在的超时时间  
	}
	debug_printf("\r\n");                        //串口输出信息
	if(timeout<=0)return 3;                   //超时错误，返回3
	else                                      //连接成功，准备进入透传
	{
		debug_printf("准备进入透传\r\n");                  //串口显示信息
		WiFi_RxCounter=0;                               //WiFi接收数据量变量清零                        
		memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);         //清空WiFi接收缓冲区     
		WiFi_printf("AT+CIPSEND\r\n");                  //发送进入透传指令
		while(timeout--){                               //等待超时与否
			vTaskDelay(100);                              //延时100ms	
			if(strstr(WiFi_RX_BUF,"\r\nOK\r\n\r\n>"))   //如果成立表示进入透传成功
				break;                          //跳出while循环
			debug_printf("%d ",timeout);           //串口输出现在的超时时间  
		}
		if(timeout<=0)return 4;                 //透传超时错误，返回4	
	}
	return 0;	                                //成功返回0	
}
/*-------------------------------------------------*/
/*函数名：连接服务器                               */
/*参  数：无                                       */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char WiFi_ConnectServer(void)
{	
	char res;
	
	debug_printf("准备复位模块\r\n");                     //串口提示数据
	if(WiFi_Reset(50)){                                //复位，100ms超时单位，总计5s超时时间
		debug_printf("复位失败，准备重启\r\n");           //返回非0值，进入if，串口提示数据
		return 1;                                      //返回1
	}else u1_printf("复位成功\r\n");                   //串口提示数据
	
	debug_printf("准备设置STA模式\r\n");                  //串口提示数据
	if(WiFi_SendCmd("AT+CWMODE=1",50)){                //设置STA模式，100ms超时单位，总计5s超时时间
		debug_printf("设置STA模式失败，准备重启\r\n");    //返回非0值，进入if，串口提示数据
		return 2;                                      //返回2
	}else debug_printf("设置STA模式成功\r\n");            //串口提示数据
	
	if(wifi_mode==0){                                      //如果联网模式=0：SSID和密码写在程序里 
		debug_printf("准备取消自动连接\r\n");                 //串口提示数据
		if(WiFi_SendCmd("AT+CWAUTOCONN=0",50)){            //取消自动连接，100ms超时单位，总计5s超时时间
			debug_printf("取消自动连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
			return 3;                                      //返回3
		}else debug_printf("取消自动连接成功\r\n");           //串口提示数据
				
		u1_printf("准备连接路由器\r\n");                   //串口提示数据	
		if(WiFi_JoinAP(200)){                               //连接路由器,1s超时单位，总计30s超时时间
			u1_printf("连接路由器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
			return 4;                                      //返回4	
		}else u1_printf("连接路由器成功\r\n");             //串口提示数据			
	}else{                                                 //如果联网模式=1：Smartconfig方式,用APP发送
//		if(KEY2_IN_STA==0){                                    //如果此时K2是按下的
//			u1_printf("准备设置自动连接\r\n");                 //串口提示数据
//			if(WiFi_SendCmd("AT+CWAUTOCONN=1",50)){            //设置自动连接，100ms超时单位，总计5s超时时间
//				u1_printf("设置自动连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
//				return 3;                                      //返回3
//			}else u1_printf("设置自动连接成功\r\n");           //串口提示数据	
//			
//			u1_printf("准备开启Smartconfig\r\n");              //串口提示数据
//			if(WiFi_SendCmd("AT+CWSTARTSMART",50)){            //开启Smartconfig，100ms超时单位，总计5s超时时间
//				u1_printf("开启Smartconfig失败，准备重启\r\n");//返回非0值，进入if，串口提示数据
//				return 4;                                      //返回4
//			}else u1_printf("开启Smartconfig成功\r\n");        //串口提示数据

//			u1_printf("请使用APP软件传输密码\r\n");            //串口提示数据
//			if(WiFi_Smartconfig(60)){                          //APP软件传输密码，1s超时单位，总计60s超时时间
//				u1_printf("传输密码失败，准备重启\r\n");       //返回非0值，进入if，串口提示数据
//				return 5;                                      //返回5
//			}else u1_printf("传输密码成功\r\n");               //串口提示数据

//			u1_printf("准备关闭Smartconfig\r\n");              //串口提示数据
//			if(WiFi_SendCmd("AT+CWSTOPSMART",50)){             //关闭Smartconfig，100ms超时单位，总计5s超时时间
//				u1_printf("关闭Smartconfig失败，准备重启\r\n");//返回非0值，进入if，串口提示数据
//				return 6;                                      //返回6
//			}else u1_printf("关闭Smartconfig成功\r\n");        //串口提示数据
//		}else{                                                 //反之，此时K2是没有按下
//			u1_printf("等待连接路由器\r\n");                   //串口提示数据	
//			if(WiFi_WaitAP(30)){                               //等待连接路由器,1s超时单位，总计30s超时时间
//				u1_printf("连接路由器失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
//				return 7;                                      //返回7	
//			}else u1_printf("连接路由器成功\r\n");             //串口提示数据					
//		}
	}
	
	u1_printf("准备获取IP地址\r\n");                   //串口提示数据
	if(WiFi_GetIP(200)){                                //准备获取IP地址，100ms超时单位，总计5s超时时间
		u1_printf("获取IP地址失败，准备重启\r\n");     //返回非0值，进入if，串口提示数据
		return 10;                                     //返回10
	}else u1_printf("获取IP地址成功\r\n");             //串口提示数据
	
	u1_printf("准备开启透传\r\n");                     //串口提示数据
	if(WiFi_SendCmd("AT+CIPMODE=1",50)){               //开启透传，100ms超时单位，总计5s超时时间
		u1_printf("开启透传失败，准备重启\r\n");       //返回非0值，进入if，串口提示数据
		return 11;                                     //返回11
	}else u1_printf("关闭透传成功\r\n");               //串口提示数据
	
	u1_printf("准备关闭多路连接\r\n");                 //串口提示数据
	if(WiFi_SendCmd("AT+CIPMUX=0",50)){                //关闭多路连接，100ms超时单位，总计5s超时时间
		u1_printf("关闭多路连接失败，准备重启\r\n");   //返回非0值，进入if，串口提示数据
		return 12;                                     //返回12
	}else u1_printf("关闭多路连接成功\r\n");           //串口提示数据
	
	u1_printf("准备连接服务器\r\n");                   //串口提示数据
	res = WiFi_Connect_Server(100);                    //连接服务器，100ms超时单位，总计10s超时时间
	if(res==1){                						   //返回1，进入if
		u1_printf("服务器未开启，准备重启\r\n");       //串口提示数据
		return 13;                                     //返回13
	}else if(res==2){                                  //返回2，进入if
		u1_printf("连接已经存在\r\n");                 //串口提示数据
	}else if(res==3){								   //返回3，进入if
		u1_printf("连接服务器超时，准备重启\r\n");     //串口提示数据
		return 14;                                     //返回14
	}else if(res==4){								   //返回4，进入if，
		u1_printf("进入透传失败\r\n");                 //串口提示数据
		return 15;                                     //返回15
	}	
	u1_printf("连接服务器成功\r\n");                   //串口提示数据
	return 0;                                          //正确返回0	
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
char  Data_buff[2048];     //数据缓冲区
void  completed_judeg(void)
{

		Usart2_RxCompleted = 1;                                       //串口2接收完成标志位置位
		memcpy(&Data_buff[2],Usart2_RxBuff,Usart2_RxCounter);         //拷贝数据
		Data_buff[0] = WiFi_RxCounter/256;                            //记录接收的数据量		
		Data_buff[1] = WiFi_RxCounter%256;                            //记录接收的数据量
		Data_buff[WiFi_RxCounter+2] = '\0';                           //加入结束符
		WiFi_RxCounter=0;                                             //清零计数值
        for(u16 i=0;i<USART2_RXBUFF_SIZE;i++)
		{
			if(Usart2_RxBuff[i] == 0)
					Usart2_RxBuff[i]=0x0d;
		}
		
}

void Usart4_RxIrqCallback(void)
{
	
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*中断屏蔽*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	
		if(Connect_flag==0)
		{                                //如果Connect_flag等于0，当前还没有连接服务器，处于指令配置状态
			if(WIFI_CH->DR){                                 //处于指令配置状态时，非零值才保存到缓冲区	
				Usart2_RxBuff[Usart2_RxCounter]= USART_RecData(WIFI_CH); //保存到缓冲区	
				Usart2_RxCounter ++;                        //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
			}		
		}else
		{		                                        //反之Connect_flag等于1，连接上服务器了	
			Usart2_RxBuff[Usart2_RxCounter] = USART_RecData(WIFI_CH);   //把接收到的数据保存到Usart2_RxBuff中			
			Usart2_RxCounter ++;         				    //每接收1个字节的数据，Usart2_RxCounter加1，表示接收的数据总量+1 
		}
	wifi_judge_num=0;
	/*中断使能*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
void wifi_analysis_handle(void)
{
	
	/*打印信息*/
//	/*转换字符串*/			
//	hex_to_ascii(usart3_commnuication_frame.analysis_buff,receive_asc_buff,usart3_commnuication_frame.analysis_long);
//	/*打印输出*/
//	debug_printf("TCP 收到数据：%s\n",receive_asc_buff);
//	//debug_printf("comm receive:%s\n",usart3_commnuication_frame.analysis_buff);
//	memset(receive_asc_buff,0,CACHE_BUFF_NUM*2);
	
	
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
#include "FreeRTOS.h"
#include "task.h"
/* Define Timer Unit for example */
#define TMR_UNIT            (M4_TMR02)
#define TMR_INI_GCMA        (INT_TMR02_GCMA)

#define ENABLE_TMR0()      (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))
uint16_t time_test=0;
void Timer0A_CallBack(void)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*中断屏蔽*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;


	wifi_judge_num++;
	if(wifi_judge_num>150)
	{
		wifi_judge_num=0;
		if(WiFi_RxCounter>0)
			completed_judeg();
			
	}  
	time_test++;	
	/*中断使能*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );			
}
void time_init(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    uint32_t u32Pclk1;
    stc_clk_freq_t stcClkTmp;
    uint32_t u32tmp;

    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);	
    /* Get pclk1 */
    CLK_GetClockFreq(&stcClkTmp);
    u32Pclk1 = stcClkTmp.pclk1Freq;

    /* Enable XTAL32 */
    CLK_Xtal32Cmd(Enable);

    /* Timer0 peripheral enable */
    ENABLE_TMR0();
    /*config register for channel A */
    stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_XTAL32;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv4;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)(32/4 - 1);
    TIMER0_BaseInit(TMR_UNIT,Tim0_ChannelA,&stcTimerCfg);


    /* Enable channel A interrupt */
    TIMER0_IntCmd(TMR_UNIT,Tim0_ChannelA,Enable);
    /* Register TMR_INI_GCMA Int to Vect.No.001 */
    stcIrqRegiConf.enIRQn = Int003_IRQn;
    /* Select I2C Error or Event interrupt function */
    stcIrqRegiConf.enIntSrc = TMR_INI_GCMA;
    /* Callback function */
    stcIrqRegiConf.pfnCallback =&Timer0A_CallBack;
    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);
    /* Clear Pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
	
    /*start timer0*/
    TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Enable);
	
	
	
}
void wifi_task(void *pvParameters)
{
	int time;
	static uint16_t clk=0;
	wifi_usart_init();
	time_init();

	
    while(WiFi_ConnectServer()){    //循环，初始化，连接服务器，直到成功
		vTaskDelay(2000);              //延时
	}      
	WiFi_RxCounter=0;  

	//WiFi接收数据量变量清零                        
	memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);    //清空WiFi接收缓冲区               
	Connect_flag = 1;                          //Connect_flag=1,表示连接上服务器	
	for(;;)
	{
		if(Usart2_RxCompleted==1){		                          //如果Usart2_RxCompleted等于1，表示接收数据完成
			Usart2_RxCompleted = 0;                               //清除标志位
			uint16_t ret= Data_buff[0]*256+Data_buff[1];
			uint8_t receive_asc_buff[200];
			hex_to_ascii(&Data_buff[2],receive_asc_buff,ret);
			u1_printf("服务器发来%d字节数据\r\n",(Data_buff[0]*256+Data_buff[1])); //串口输出信息
			u1_printf("数据:%s\r\n",receive_asc_buff);               //串口输出信息   
			WiFi_printf("您发送的数据数:%s\r\n",&Data_buff[2]);	  //把接收到的数据，返回给服务器
			memset(receive_asc_buff,0,sizeof(receive_asc_buff));
	    }
		if(time>=1000){                                           //当time大于等于1000的时候，大概经过1s的时间
			time=0;                                               //清除time计数
			WiFi_printf("我是客户端，请发送数据!\r\n");           //给服务器发送数据
		}
		
		time++;       //time计数器+1
		clk++; 			
		/*打印信息*/		
		if((clk%300)==0)
			debug_printf("wifi task runing:%d\n",clk/100);		
		vTaskDelay(3);
		//common_idle_judge(&wifi_usart2_frame);
		//data_analysis_handle(&wifi_usart2_frame,wifi_analysis_handle);
	}
}

/***********************************************END*****************************************************/

