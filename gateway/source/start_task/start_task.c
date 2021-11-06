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
#include "start_task.h"
#include "broker_task.h"
#include "button_business.h"
#include "rs458_task.h"
#include "ethernet_task.h"
#include "logic_control_task.h"
#include "gpio_manage_business.h"
#include "data_task.h"
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
TaskHandle_t StartTask_Handler;

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void debug_task(void *pvParameters);
void debug_usart_init(void);
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void system_clk_init(void )
{
    stc_clk_xtal_cfg_t   stcXtalCfg;
    stc_clk_mpll_cfg_t   stcMpllCfg;
    en_clk_sys_source_t  enSysClkSrc;
    stc_clk_sysclk_cfg_t stcSysClkCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Switch system clock source to MPLL. */
    /* Use Xtal as MPLL source. */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    CLK_HrcCmd(Enable);
    while (Set != CLK_GetFlagStatus(ClkFlagHRCRdy))
    {
    }	
    /* MPLL config. */
    stcMpllCfg.pllmDiv = 3; /* XTAL 12M / 3 */
    stcMpllCfg.plln = 100;   /* 4M*100 = 400M */
    stcMpllCfg.PllpDiv = 2u; /* MLLP = 200M */
    stcMpllCfg.PllqDiv = 4u; /* MLLQ = 100M */
    stcMpllCfg.PllrDiv = 4u; /* MLLR = 100M */
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_4);
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);

    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
    }

    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
    /* 嘀嗒系统时钟 */	
	SysTick_Init(1000u);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();          /*进入临界区*/
	/* 配置系统时钟 */
	system_clk_init();
	/* DEBUG初始化 */
	debug_usart_init();
	/*创建代理者任务*/					
	xTaskCreate((TaskFunction_t )broker_task,    
				(const char*    )"broker",  
				(uint16_t       )BROKER_STK_SIZE,
				(void*          )NULL,
				(UBaseType_t    )BROKER_TASK_PRIO,
				(TaskHandle_t*  )NULL);	
	/*创建485任务*/					
	xTaskCreate((TaskFunction_t )rs485_task,    
				(const char*    )"rs485",  
				(uint16_t       )RS485_STK_SIZE,
				(void*          )NULL,
				(UBaseType_t    )RS485_TASK_PRIO,
				(TaskHandle_t*  )NULL);	
				
	/*创建以太网任务*/					
	xTaskCreate((TaskFunction_t )ethernet_task,    
				(const char*    )"ethernet",  
				(uint16_t       )ETHERNET_STK_SIZE,
				(void*          )NULL,
				(UBaseType_t    )ETHERNET_TASK_PRIO,
				(TaskHandle_t*  )NULL);		
	/*创建用户逻辑控制任务*/					
	xTaskCreate((TaskFunction_t )logic_control_task,    
				(const char*    )"logic_",  
				(uint16_t       )LOGIC_STK_SIZE,
				(void*          )NULL,
				(UBaseType_t    )LOGIC_TASK_PRIO,
				(TaskHandle_t*  )NULL);	
	/*创建输出控制任务*/					
	xTaskCreate((TaskFunction_t )control_task,    
				(const char*    )"control",  
				(uint16_t       )CONTROL_STK_SIZE,
				(void*          )NULL,
				(UBaseType_t    )CONTROL_TASK_PRIO,
				(TaskHandle_t*  )NULL);	
	extern u8 firmware_version ;
	debug_printf("%s%s%s%s%d%s%s",
	"\r\n",
	"************************网关启动*************************\n",
	"************************用户固件*************************\n",
	"************************固件版本V0.",firmware_version,"*********************\n"	
	"*********************************************************\n");	
	vTaskDelay(10);/*延时 系统切换任务*/				
    vTaskDelete(NULL); 				/*删除开始任务*/					
    taskEXIT_CRITICAL();            /*退出临界区*/	
}

/***********************************************END*****************************************************/

