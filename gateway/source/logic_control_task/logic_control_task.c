
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
#include "hc32f46x_clk.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logic_control_task.h"
#include "broker_task.h"
#include "user_type.h"
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
typedef void (*interrupt_callback)(void);

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
/*业务订阅发布结构数据*/
business_information_t  logic_business=
{
	.release_name = "logic",
	.subscribe_name = "rs485inside,rs485outside,gpio,ethernet,local_ethernet",		
	.release_data.serial =0,
	.large_data_length = 0,
};

key_value_frame_t user_key_data;
key_value_frame_t user_key_analysis;
key_value_frame_t user_key_execute;

key_value_delay_frame_t  user_delay_key;
static data_container_t  current_data_container;

system_information_t user_system_data;

release_buff_t logic_business_release_buff=
{
	.pointer = 0,
};
u8 *user_system_data_point =  (u8*)&user_system_data;
user_total_information_t  current_user;
u8 key_value_updata_flag =0;
extern u16 ethernet_data_monitor_num ;
user_logic_t user_logic;
u8 user_uid[3];
extern u8 firmware_version ;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void communication_485_analysis(data_container_t* data_container);
static void dry_contac_input_analysis(data_container_t* data_container);
void ethernet_send_hearbeat(data_container_t* current_data_container);
void id_transformation(u8 *buff, u8 *id_buff);
void ethernet_data_analysis(data_container_t* data_container ,data_container_t* current_data_container);
void key_value_analysis_form_subscribe(data_container_t* data_container);
void communication_485_handle(u8* key_buff ,u8 type, data_container_t  *current_data_container);
static void delay_data_release(void);
void wdt_config(void);
void read_uid_data(void);
void logic_control_time_int(interrupt_callback user_handle);
void logic_control_interrupt_callback(void );
void analysis_of_air_conditioning_instructions(data_container_t* data_container);
void key_value_logic_handle(void );
void key_card_power_off(void);

/**
*********************************************************************************************************
* @名称	: 
* @描述	:控制整个用户任务的逻辑 ，纯数据处理，无硬件操作
*********************************************************************************************************
**/
void logic_control_task(void *pvParameters)
{
	static uint16_t clk=0;
	/*从FLAHS读取系统数据*/
	user_read_system_data(); 
	/*读取UID*/
	read_uid_data();
	/*用户总数据清零*/	
	key_buff_clear();
	memset(&current_user, 0,sizeof(current_user));
	/*初始化定时 并注册中断回调函数*/
	logic_control_time_int( logic_control_interrupt_callback );
	/*看门狗配置*/
	wdt_config();
	/*关掉所有设备*/
	key_card_power_off();	
	while(1)
	{		
		/*键值解析并缓存键值*/
		key_value_analysis_cache();
		/*键值逻辑处理*/
		key_value_logic_handle();		
		/*执行键值*/		
		key_value_execute();	
		/*以太网发送心跳数据*/
		ethernet_send_hearbeat(&current_data_container);
		/*喂狗*/		
		WDT_RefreshCounter();
		/*打印信息*/	
		if((clk++%3000) == 0) 
			debug_printf("逻辑控制任务运行中...\n");	
		/*延时*/
		vTaskDelay(3);		
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:逻辑控制中断回调函数 100ms中断 处理秒级延时 
*********************************************************************************************************
**/
void logic_control_interrupt_callback(void)
{
	/*延时按键扫描查询*/
	key_delay_scanf_handle();
	/*从缓存中发布数据并延时*/
	delay_data_release();	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 读取UID   
*********************************************************************************************************
**/
void read_uid_data(void)
{
	u32 num;
	num = M4_EFM->UQID3;
	user_uid[2] = num>>16;
	user_uid[1] = num>>8;
	user_uid[0] = num;		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:从缓存中发布数据并延时
*********************************************************************************************************
**/
static void delay_data_release(void)
{
	for(u8 i = 0 ; i < RELEASE_BUFF_MAX ; i++)
	{
		/*判断缓存是否有数据*/
		if(logic_business_release_buff.release[i].serial != 0)
		{
			/*判断延时*/
			if(logic_business_release_buff.delay[i] > 0 )
			{
				logic_business_release_buff.delay[i]--;
			}
			else
			{
				/*判断发布数据是否为空 只有为空时才操作否则会覆写*/
				if(logic_business.release_data.serial == 0) 
				{
					memcpy(logic_business.release_name, &logic_business_release_buff.release[i].name, sizeof(logic_business.release_data.name)); /*更改发布名字*/
					/*复制数据*/
					memcpy(logic_business.release_data.name, &logic_business_release_buff.release[i].name, sizeof(logic_business.release_data.name));
					memcpy(logic_business.release_data.buff, &logic_business_release_buff.release[i].buff, sizeof(logic_business.release_data.buff));
					logic_business.release_data.length = logic_business_release_buff.release[i].length;
					logic_business.release_data.serial = logic_business_release_buff.release[i].serial;
					/*清零数据 */
					memset(&logic_business_release_buff.release[i], 0, sizeof(data_container_t));
					/*完成发送就跳出 避免多次判断发送 */
					break;
				}							
			}	
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值解析
*********************************************************************************************************
**/
static void key_value_analysis_cache(void)
{
	/*读取订阅数据并执行回调函数*/
	read_subscribe_payload(&logic_business , key_value_analysis_form_subscribe);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:解析订阅信息中的键值 
*********************************************************************************************************
**/
void key_value_analysis_form_subscribe(data_container_t* data_container)
{
	/*485 键值解析*/
	communication_485_analysis(data_container);
	/*插卡判断 */
	dry_contac_input_analysis(data_container);/*干接点键值解析*/
	/*以太网数据解析*/
	ethernet_data_analysis(data_container, &current_data_container);	
	/*解析空调指令*/
	analysis_of_air_conditioning_instructions(data_container);			
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:干接点输入
*********************************************************************************************************
**/
static void dry_contac_input_analysis(data_container_t* data_container)
{
	u8 key_buff[3];
	if( ( strstr("gpio"  ,(char*)data_container->name ) ) != NULL )  /*判断发布名是否为button*/
	{
		/*buff[0]通道   buff[1] 高低电平*/
		u8 i = data_container->buff[0];  /*按键通道   增加类型判断*/	
		/*一旦按键发送变化 必然触发 自恢复*/
		//if((user_system_data.logic.input[i].type == 1) || (user_system_data.logic.input[i].type == 3))
		/*自恢复低 一旦按键发送变化 必然触发*/
		if( user_system_data.logic.input[i].type == 1 )
		{
			if( data_container->buff[1] == 0)
			{
				/*缓存3字节键值*/
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0;/*取反*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;				
				key_buff_write(key_buff, 0);
			}				
		}
		/*自恢复高 一旦按键发送变化 必然触发*/
		else if(user_system_data.logic.input[i].type == 3 )
		{
			if( data_container->buff[1] == 1)
			{
				/*缓存3字节键值*/
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0;/*取反*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;				
				key_buff_write(key_buff, 0);
			}		
		}			
		/*高有效 绝对开关*/
		else if(user_system_data.logic.input[i].type == 0 )
		{
			if( data_container->buff[1] == 1)
			{		
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 1;/*1 开*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);
			}
			else
			{			
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 2;/*2 关*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);					
			}
		}
		/*低有效 绝对开关*/
		else if(user_system_data.logic.input[i].type == 2 )
		{
			if( data_container->buff[1] == 0)
			{
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 1;/*1 开*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);
			}
			else
			{
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 2;/*2 关*/
				key_buff[DATA_BYTE] =user_system_data.logic.input[i].value;
				key_buff_write(key_buff, 0);					
			}
		}			
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值处理
*********************************************************************************************************
**/
static void key_value_execute(void)
{
	u8 key_buff[3],type;
	/*键值缓存不为空*/
	if(key_buff_check())
	{
		/*键值更新立即上报云端*/
		key_value_updata_flag = 1;
		/*读取键值和类型*/
		type = key_buff_read(key_buff);
		/*通讯键值处理*/
		communication_485_handle(key_buff, type , &current_data_container);
		/*输出键值处理*/
		output_control_handle(key_buff);	
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:输出控制   使用键值BUFF[0] 存一个延时键值 同时  BUFF[2] 为FF
*********************************************************************************************************
**/
static void output_control_handle(u8* key_buff)
{
	for(u8 i=0; i < 10; i++)
	{
		if(compare_hex_buff(&key_buff[DATA_BYTE], &user_system_data.logic.output[i] ,1))/*  不比较 0位标记位  */
		{
			
			memcpy(current_data_container.name, "logic_output", sizeof("logic_output"));
			
			current_data_container.buff[0] =i;
			current_data_container.buff[1] =key_buff[STATUS_BYTE];
			
			current_data_container.length = 2;
			current_data_container.serial = 1;	
			release_buff_write( &logic_business_release_buff, &current_data_container, 5);	
			break;
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:用户数据初始化
*********************************************************************************************************
**/
void uesr_data_reset(void)
{
	memcpy(user_system_data.set.domain_name,  "admin.hotelyun.net", sizeof("admin.hotelyun.net"));
	user_system_data.set.domain_name_length = sizeof("admin.hotelyun.net");
	user_system_data.set.domain_name_port[0] = 5188/256;
	user_system_data.set.domain_name_port[1] = 5188%256;
	
	user_system_data.set.load_balancing_ip[0] = 47;
	user_system_data.set.load_balancing_ip[1] = 106;
	user_system_data.set.load_balancing_ip[2] = 234;
	user_system_data.set.load_balancing_ip[3] = 88;
	
	user_system_data.set.load_balancing_port[0] = 5188/256;
	user_system_data.set.load_balancing_port[1] = 5188%256;	
	
	for(u8 i = 0 ; i<10 ; i++)
	user_system_data.check[i] = i;
}
/***********************************************END*****************************************************/

