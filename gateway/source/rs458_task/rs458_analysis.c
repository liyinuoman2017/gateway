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
#include "rs458_bsp.h"
#include "rs458_analysis.h"

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
usart_frame_t   usart3_commnuication_frame;   /*insied*/
usart_frame_t   usart4_commnuication_frame;   /*outsied*/
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/


/**
*********************************************************************************************************
* @名称	: 
* @描述	: 刷新串口延时,存放接收数据
*********************************************************************************************************
**/
void common_receive_refresh(uint8_t receive_data, usart_frame_t *com_pointer)
{
	//刷新计数器
	com_pointer->refresh_num=0;
	//缓存数据
	if(com_pointer->current_num<(CACHE_BUFF_NUM-1))
		com_pointer->receive_buff[com_pointer->current_num++]=receive_data;		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 超过10MS，串口进入空闲 
*********************************************************************************************************
**/
void common_idle_judge(usart_frame_t *com_pointer)
{
	com_pointer->refresh_num++;
	if(com_pointer->refresh_num>=IDLE_NUM)
	{
		com_pointer->refresh_num=IDLE_NUM;
		if(com_pointer->current_num>0)
		{	
			//缓存数据
			common_cache_data(com_pointer->current_num,com_pointer);
			com_pointer->current_num=0;
		}		
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 缓存串口数据
*********************************************************************************************************
**/
void common_cache_data(uint16_t buff_long,usart_frame_t *com_pointer)
{
	//判断缓存队列是否满
	if(com_pointer->data_pointer<(CACHE_BUFF_LONGTH-1))
	{
		//拷贝数据
		memcpy(com_pointer->data_cache[com_pointer->data_pointer],com_pointer->receive_buff,sizeof(com_pointer->data_cache[0]));		
		//保存数据长度
		com_pointer->data_cache_long[com_pointer->data_pointer]=buff_long;
		//数据序号加一
		com_pointer->data_pointer++;		
	}
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 接收数据解析，执行无形参无返回函数 
*********************************************************************************************************
**/
void data_analysis_handle(usart_frame_t* com_pointer,fun_void  common_analysis_handle)
{
	if(com_pointer->data_pointer>0)
	{
			
		//清空解析BUFF
		memset(com_pointer->analysis_buff,0,sizeof(com_pointer->analysis_buff));
		//从缓存中拷贝数据到解析BUFF
		memcpy(com_pointer->analysis_buff,com_pointer->data_cache[com_pointer->data_pointer-1],sizeof(com_pointer->analysis_buff));
		//拷贝数据长度
		com_pointer->analysis_long= com_pointer->data_cache_long[com_pointer->data_pointer-1];
		//数据解析		
		(*common_analysis_handle)();
		// 清空缓存
		memset(com_pointer->data_cache[com_pointer->data_pointer-1],0,sizeof(com_pointer->data_cache[0]));	
		com_pointer->data_cache_long[com_pointer->data_pointer-1]=0;

		com_pointer->data_pointer--;				
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 串口接收中断回调函数 
*********************************************************************************************************
**/
void UsartRx3IrqCallback(void)
{
	/*insied*/
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*中断屏蔽*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	/*接收数据 存入缓存*/
    data = USART_RecData(USART_CH);
	common_receive_refresh(data,&usart3_commnuication_frame);
	/*中断使能*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 串口接收中断回调函数 
*********************************************************************************************************
**/
void UsartRx4IrqCallback(void)
{
	/*outsied*/
	uint16_t  data;	
	static portBASE_TYPE xHigherPriorityTaskWoken;	
	/*中断屏蔽*/
	__disable_irq();	
	xHigherPriorityTaskWoken = pdFALSE;
	/*接收数据 存入缓存*/
    data = USART_RecData(USART_CH2);
	common_receive_refresh(data,&usart4_commnuication_frame);
	/*中断使能*/	
	__enable_irq();
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void Timer6_OverFlow_CallBack(void)
{
	common_idle_judge(&usart3_commnuication_frame);
	common_idle_judge(&usart4_commnuication_frame);
}

/***********************************************END*****************************************************/


