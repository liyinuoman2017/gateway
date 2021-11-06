
/**
*********************************************************************************************************
*                                        		gateway
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.02
* 
*********************************************************************************************************
**/

/*
*********************************************************************************************************
Includes 
*********************************************************************************************************
*/
#include "hc32_ddl.h"

/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define TMR_UNIT            (M4_TMR02)
#define TMR_INI_GCMA        (INT_TMR02_GCMA)

#define ENABLE_TMR0()      (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))

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
typedef void (*interrupt_callback)(void);
interrupt_callback timer_callback;

/**
*********************************************************************************************************
* @名称	: 
* @描述	:10ms中断  用于秒计算
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
void Timer0A_CallBack(void)
{
	/*调用中断回调函数 */
	timer_callback();
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:硬件初始化
*********************************************************************************************************
**/
void logic_control_time_int(interrupt_callback user_handle)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_irq_regi_conf_t stcIrqRegiConf;	
    /* Enable XTAL32 */
    CLK_Xtal32Cmd(Enable);

    /* Timer0 peripheral enable */
    ENABLE_TMR0();
    /*config register for channel A */
    stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv4;
    stcTimerCfg.Tim0_CmpValue = (uint16_t)(320/4 - 1);
    TIMER0_BaseInit(TMR_UNIT,Tim0_ChannelA,&stcTimerCfg);

    /* Enable channel A interrupt */
    TIMER0_IntCmd(TMR_UNIT,Tim0_ChannelA,Enable);
    /* Register TMR_INI_GCMA Int to Vect.No.001 */
    stcIrqRegiConf.enIRQn = Int001_IRQn;
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
	
    TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Enable);
	/*注册中断回调函数 */	
	timer_callback = user_handle;

}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:看门狗设置  约3秒复位 
*********************************************************************************************************
**/
void wdt_config(void)
{
    stc_wdt_init_t stcWdtInit;
    MEM_ZERO_STRUCT(stcWdtInit);
    stcWdtInit.enClkDiv = WdtPclk3Div2048;
    stcWdtInit.enCountCycle = WdtCountCycle16384;
    stcWdtInit.enRefreshRange = WdtRefresh0To100Pct;
    stcWdtInit.enSleepModeCountEn = Disable;
    stcWdtInit.enRequestType = WdtTriggerResetRequest;
    WDT_Init(&stcWdtInit);
	WDT_RefreshCounter();/*刷计数 开始计数*/		
}
/***********************************************END*****************************************************/

