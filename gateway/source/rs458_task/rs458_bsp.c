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
#include "rs458_bsp.h"

/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
///* USART channel definition */
//#define USART_CH                        (M4_USART3)

/* USART baudrate definition */
#define USART_BAUDRATE                  (9600)

/* USART RX Port/Pin definition */
#define USART_RX_PORT                   (PortB)
#define USART_RX_PIN                    (Pin13)
#define USART_RX_FUNC                   (Func_Usart3_Rx)

/* USART TX Port/Pin definition */
#define USART_TX_PORT                   (PortB)
#define USART_TX_PIN                    (Pin12)
#define USART_TX_FUNC                   (Func_Usart3_Tx)

/* USART interrupt number  */
#define USART_RI_NUM                    (INT_USART3_RI)
#define USART_EI_NUM                    (INT_USART3_EI)
#define USART_TI_NUM                    (INT_USART3_TI)
#define USART_TCI_NUM                   (INT_USART3_TCI)


#define RS485EN_TX     PORT_SetBits(PortB, Pin10);
#define RS485EN_RX     PORT_ResetBits(PortB, Pin10);



///* USART channel definition */
//#define USART_CH2                        (M4_USART4)

/* USART baudrate definition */


/* USART RX Port/Pin definition */
#define USART_RX_PORT2                   (PortB)
#define USART_RX_PIN2                    (Pin15)
#define USART_RX_FUNC2                   (Func_Usart4_Rx)

/* USART TX Port/Pin definition */
#define USART_TX_PORT2                   (PortB)
#define USART_TX_PIN2                    (Pin14)
#define USART_TX_FUNC2                   (Func_Usart4_Tx)

/* USART interrupt number  */
#define USART_RI_NUM2                    (INT_USART4_RI)
#define USART_EI_NUM2                    (INT_USART4_EI)
#define USART_TI_NUM2                    (INT_USART4_TI)
#define USART_TCI_NUM2                   (INT_USART4_TCI)


#define RS485EN_TX2     PORT_SetBits(PortB, Pin02);
#define RS485EN_RX2     PORT_ResetBits(PortB, Pin02);

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
void UsartRx3IrqCallback(void);
void UsartRx4IrqCallback(void);
void Timer6_OverFlow_CallBack(void);
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	:  
*********************************************************************************************************
**/
static void Usart3_ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(USART_CH, UsartFrameErr))
    {
        USART_ClearStatus(USART_CH, UsartFrameErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(USART_CH, UsartParityErr))
    {
        USART_ClearStatus(USART_CH, UsartParityErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(USART_CH, UsartOverrunErr))
    {
        USART_ClearStatus(USART_CH, UsartOverrunErr);
    }
    else
    {
    }
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	:  
*********************************************************************************************************
**/
static void Usart4_ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(USART_CH2, UsartFrameErr))
    {
        USART_ClearStatus(USART_CH2, UsartFrameErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(USART_CH2, UsartParityErr))
    {
        USART_ClearStatus(USART_CH2, UsartParityErr);
    }
    else
    {
    }

    if (Set == USART_GetStatus(USART_CH2, UsartOverrunErr))
    {
        USART_ClearStatus(USART_CH2, UsartOverrunErr);
    }
    else
    {
    }
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	:  
*********************************************************************************************************
**/
#define USART_DIV  UsartClkDiv_4
void usart3_init(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        USART_DIV,
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
    PORT_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_FUNC, Disable);
    PORT_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_FUNC, Disable);

    /* Initialize UART */
    enRet = USART_UART_Init(USART_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    /* Set baudrate */
    enRet = USART_SetBaudrate(USART_CH, USART_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int000_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartRx3IrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	
    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = Int005_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart3_ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    /*Enable RX && RX interupt function*/
    USART_FuncCmd(USART_CH, UsartRx, Enable);
    USART_FuncCmd(USART_CH, UsartRxInt, Enable);
	
	USART_FuncCmd(USART_CH, UsartTxAndTxEmptyInt, Enable);


	//////485 EN
    stc_port_init_t stcPortInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_Init(PortB,  Pin10, &stcPortInit);	
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	: 
*********************************************************************************************************
**/
void usart4_init(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 | PWC_FCG1_PERIPH_USART2 | \
                             PWC_FCG1_PERIPH_USART3 | PWC_FCG1_PERIPH_USART4;
    const stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        USART_DIV,
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
    PORT_SetFunc(USART_RX_PORT2, USART_RX_PIN2, USART_RX_FUNC2, Disable);
    PORT_SetFunc(USART_TX_PORT2, USART_TX_PIN2, USART_TX_FUNC2, Disable);

    /* Initialize UART */
    enRet = USART_UART_Init(USART_CH2, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }
    /* Set baudrate */
    enRet = USART_SetBaudrate(USART_CH2, USART_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int006_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartRx4IrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_RI_NUM2;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
	
    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = Int007_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4_ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_EI_NUM2;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    /*Enable RX && RX interupt function*/
    USART_FuncCmd(USART_CH2, UsartRx, Enable);
    USART_FuncCmd(USART_CH2, UsartRxInt, Enable);
	
	USART_FuncCmd(USART_CH2, UsartTxAndTxEmptyInt, Enable);


	//////485 EN
    stc_port_init_t stcPortInit;
    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_Init(PortB,Pin02 , &stcPortInit);	
	RS485EN_RX2;
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	: 
*********************************************************************************************************
**/
void time6_init(void)
{
    stc_timer6_basecnt_cfg_t         stcTIM6BaseCntCfg;
    stc_timer6_port_output_cfg_t     stcTIM6PWMxCfg;
    stc_timer6_gcmp_buf_cfg_t        stcGCMPBufCfg;
    stc_irq_regi_conf_t              stcIrqRegiConf;

    MEM_ZERO_STRUCT(stcTIM6BaseCntCfg);
    MEM_ZERO_STRUCT(stcTIM6PWMxCfg);
    MEM_ZERO_STRUCT(stcGCMPBufCfg);

    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM61, Enable);   //Enable Timer61 Module
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);     //Enable AOS Module
       
    stcTIM6BaseCntCfg.enCntMode   = Timer6CntSawtoothMode;              //Sawtooth wave mode
    stcTIM6BaseCntCfg.enCntDir    = Timer6CntDirUp;                     //Counter counting up
    stcTIM6BaseCntCfg.enCntClkDiv = Timer6PclkDiv1;                     //Count clock: pclk0
    Timer6_Init(M4_TMR61, &stcTIM6BaseCntCfg);                          //timer6 PWM frequency, count mode and clk config
    
    M4_DBGC->MCUSTPCTL_f.TM61STP = 1;
    M4_DBGC->MCUSTPCTL_f.TM62STP = 1;
    Timer6_SetPeriod(M4_TMR61, Timer6PeriodA, 8200);            //Timer61 period set
    Timer6_SetTriggerSrc0(EVT_TMR61_GOVF);            //Set Timer61 OVF envet as Timer6 AOS0 event.
            
    /*config interrupt*/
    /* Enable timer61 GOVF interrupt */
    Timer6_ConfigIrq(M4_TMR61, Timer6INTENOVF, true);
    
    stcIrqRegiConf.enIRQn = Int002_IRQn;                    //Register INT_TMR62_GOVF Int to Vect.No.002
    stcIrqRegiConf.enIntSrc = INT_TMR61_GOVF;               //Select Event interrupt of timer61
    stcIrqRegiConf.pfnCallback = &Timer6_OverFlow_CallBack;  //Callback function
    enIrqRegistration(&stcIrqRegiConf);                     //Registration IRQ

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);            //Clear Pending
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);//Set priority
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);                   //Enable NVIC
    /*start timer6*/
    Timer6_StartCount(M4_TMR61);

}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	: 
*********************************************************************************************************
**/
void rs485_usart_init(void)
{
	usart3_init();
	usart4_init();
	time6_init();
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	:  
*********************************************************************************************************
**/
void rs485_inside_send_buff(uint8_t* buff,uint8_t length)
{
	/*¹Ø×ÜÖÐ¶Ï*/
	__disable_irq();	
	RS485EN_TX;
	for(uint8_t i = 0; i < length; i++)
	{
		while (Reset == USART_GetStatus(USART_CH, UsartTxEmpty));
		USART_SendData(USART_CH, buff[i]);
		while (Reset == USART_GetStatus(USART_CH, UsartTxComplete)); 
		USART_ClearStatus(USART_CH, UsartTxComplete);
	}
	RS485EN_RX;
	/*¿ª×ÜÖÐ¶Ï*/	
	__enable_irq();	
}
/**
*********************************************************************************************************
* @Ãû³Æ	: 
* @ÃèÊö	: 
*********************************************************************************************************
**/
void rs485_outside_send_buff(uint8_t* buff,uint8_t length)
{
	RS485EN_TX2;
	for(uint8_t i = 0; i < length; i++)
	{
		while (Reset == USART_GetStatus(USART_CH2, UsartTxEmpty));
		USART_SendData(USART_CH2, buff[i]);
		while (Reset == USART_GetStatus(USART_CH2, UsartTxComplete)); 
		USART_ClearStatus(USART_CH2, UsartTxComplete);
	}
	RS485EN_RX2;
}
/***********************************************END*****************************************************/

