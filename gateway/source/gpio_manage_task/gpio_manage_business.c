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
#include "debug_bsp.h"
#include "broker_task.h"
#include "user_type.h"
#include "gpio_manage_business.h"
#include "gpio_manage_driver.h"
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
TaskHandle_t control_task_handler;

business_information_t  gpio_business=
{
	.release_name = "gpio",
	.subscribe_name = "logic_output",		
	.release_data.serial =0,
	.large_data_length = 0,
};
u8 lamp_status[10]={0,0,0,0,0,0,0,0,0};
u8 button_status[10]={0,0,0,0,0,0,0,0,0};
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void control_lamp(uint8_t channel, uint8_t value);
void  switch_driver_interface(u8 cmd ,u8* data);
void switch_handle(void);
void switch_read_subscribe(data_container_t* business);
void button_handle(u8 *buff);
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void control_task(void *pvParameters)
{
	static uint16_t clk=0;
	static uint8_t led_state=0;
	static u8 read_buff[5];
	/*初始化*/
	gpio_driver_init();
	while(1)
	{
		/*读取订阅数据并执行回调函数*/
		read_subscribe_payload(&gpio_business , switch_read_subscribe);
		/*按键扫描*/
		button_scanf();
		if(read_single(read_buff))
		{
			button_handle(read_buff);
		}
		/*打印信息*/
		if(((clk++)%1000) == 0) 
		{
//			debug_printf("%s%s%s%d%s%d%s%s%d%s%d%s%s%d%s%d%s%s%d%s%d%s%s%d%s%d%s%s",
//			"******************IO控制任务运行状态*********************\n",
//			"*输出通道\t输出状态\t 按键通道\t按键状态*\n",
//			"*1\t\t", lamp_status[0],"\t\t 1\t\t",button_status[0],"\t*\n",
//			"*2\t\t", lamp_status[1],"\t\t 2\t\t",button_status[1],"\t*\n",
//			"*3\t\t", lamp_status[2],"\t\t 3\t\t",button_status[2],"\t*\n",
//			"*4\t\t", lamp_status[3],"\t\t 4\t\t",button_status[3],"\t*\n",
//			"*5\t\t", lamp_status[4],"\t\t 5\t\t",button_status[4],"\t*\n",
//			"*********************************************************\n");	
			debug_printf("IO控制任务运行\n");/*打印信息*/	
		}
		/*打印信息*/
		if(((clk++)%49) == 0)
		{
			if(((led_state++)%2) == 0) 
				switch_driver_write(0 , 5 , 1);	
			else
				switch_driver_write(0 , 5 , 0);		
		}			
		vTaskDelay(10);/*延时 系统切换任务*/			
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: buff[0] 通道  buff[1]绝对开关  0取反  1 开 2 关 
*********************************************************************************************************
**/
void switch_read_subscribe(data_container_t* data_container)
{
	if(data_container->buff[1] == 0)
		lamp_status[data_container->buff[0]]= !lamp_status[data_container->buff[0]];/*相应通道 数据取反*/
	else if(data_container->buff[1] == 1)
		lamp_status[data_container->buff[0]]= 1;
	else if(data_container->buff[1] == 2)
		lamp_status[data_container->buff[0]]= 0;	

	switch_handle();/*执行开关操作*/	
	debug_printf("*IO控制任务:通道%d输出%d\r\n", data_container->buff[0]+1, lamp_status[data_container->buff[0]]); 
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void switch_handle(void)
{
	/*依次刷新8个灯状态*/
	for(uint8_t i = 0; i < 8; i++)	
		switch_driver_write(0 , i, lamp_status[i]);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
void button_handle(u8 *buff)
{

	gpio_business.release_data.buff[0] = buff[0];  /*通道 1~7*/
	gpio_business.release_data.buff[1] = buff[1];  /*类型*/
	/*发布消息 先写数据后写序列号*/
	gpio_business.release_data.serial = 1;	
	debug_printf("*IO控制任务:按键%d 触发,类型%d \r\n", buff[0], buff[1]);
	/*保存按键状态*/
	button_status[buff[0]] = buff[1];

}
/***********************************************END*****************************************************/

