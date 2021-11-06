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
#include "broker_task.h"
#include "user_type.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define BROKER_AMOUNT   4

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
extern business_information_t  rs485_business;
extern business_information_t  logic_business;
extern business_information_t  gpio_business;
extern business_information_t  ethernet_business;
/*代理信息一定要填写正确，数量一定要正确，不然程序跑飞*/
business_information_t* broker_information[BROKER_AMOUNT] =
{
	&rs485_business,
	&logic_business,	
	&gpio_business,
	&ethernet_business,
};
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void clear_release_information(data_container_t* release);
void broker_push_information(u8 release);
void broker_handle(void);
void clear_broker_information(void);

/**
*********************************************************************************************************
* @名称	: 
* @描述	:代理者任务，复制读取每个业务的发布数据，并将数据推送给相关订阅者  
*********************************************************************************************************
**/
void broker_task(void *pvParameters)
{
	clear_broker_information();/*清空代理业务的信息 */
	while(1)
	{
		broker_handle();/*代理者信息推送处理*/
		vTaskDelay(1); 
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空代理业务的信息 
*********************************************************************************************************
**/
static void clear_broker_information(void)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++ ) /*依次清空所有业务数据*/
	{
		/*清空发布消息和序列号*/
		memset( broker_information[i]->release_data.buff, 0 , sizeof( broker_information[0]->release_data.buff ) );
		broker_information[i]->release_data.serial = 0;	
		/*清空订阅消息和序列号*/
		for( u8 k = 0; k < (SUBSCRIBE_AMOUNT - 1); k++ )
		{
			memset( broker_information[i]->subscribe_data_buff[ k ].buff, 0 , sizeof( broker_information[0]->subscribe_data_buff[ 0 ].buff ) );
			broker_information[i]->subscribe_data_buff[ k ].serial = 0;	
		}			
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:代理者事情处理，推送信息
*********************************************************************************************************
**/
static void broker_handle(void)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++ ) /*依次轮询所有业务*/
	{
		if( broker_information[i]->release_data.serial != 0 ) /*判断该业务发布序列是否不为0*/	
		{
			/*推送消息给订阅者*/
			broker_push_information( i );
			/*清空发布内容*/
			clear_release_information(&broker_information[i]->release_data);
		}	
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:推送信息给订阅者 
*********************************************************************************************************
**/
static void broker_push_information(u8 release)
{
	for( u8 i = 0; i < BROKER_AMOUNT; i++)  /*依次轮询所有业务*/	
	{
		if( i != release) /*将消息推送给其他业务*/
		{
			if( ( strstr((char*)broker_information[i]->subscribe_name  ,(char*)broker_information[release]->release_name ) ) != NULL )  /*判断发布名是否在订阅名中*/
			{
				for( u8 k = 0; k < (SUBSCRIBE_AMOUNT - 1); k++ )/*依次轮询所有订阅缓存是否有空位*/
				{
					if( broker_information[i]->subscribe_data_buff[k].serial == 0 ) /*判断订阅缓序列号是否为0 ，0为空*/
					{
						/*复制数据 */
						memcpy( broker_information[i]->subscribe_data_buff[k].buff, broker_information[release]->release_data.buff ,CONTAINER_AMOUNT);						
						/*名字*/
						memcpy( broker_information[i]->subscribe_data_buff[k].name, broker_information[release]->release_name , sizeof(broker_information[0]->subscribe_data_buff[0].name));						
						/*序列号*/
						broker_information[i]->subscribe_data_buff[k].serial = broker_information[release]->release_data.serial;
						/*数据长度*/
						broker_information[i]->subscribe_data_buff[k].length = broker_information[release]->release_data.length;	
						/*复制超长数据数据*/
						if((broker_information[i]->large_data_length != 0 )&& ( broker_information[i]->large_data_length >=broker_information[release]->large_data_user_length))	
							memcpy(broker_information[i]->large_data_buff ,broker_information[release]->large_data_buff,broker_information[release]->large_data_user_length);	
						/*长数据数据清零，就绪*/
						broker_information[release]->large_data_user_length = 0;
						break;
					}
				}		
			}										
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空信息 
*********************************************************************************************************
**/
void clear_release_information(data_container_t* release)
{
	/*清空发布消息和序列号*/
	memset( release->buff, 0, sizeof( release->buff ));
	memset( release->name, 0, sizeof( release->name ));	
	release->serial = 0;
	release->length = 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:检测订阅信息 
*********************************************************************************************************
**/
u8 check_subscribe_information(business_information_t* business)
{
	for(u8 i=0 ; i<(SUBSCRIBE_AMOUNT - 1) ;i++)
	{
		if(business->subscribe_data_buff[i].serial !=0) /*检测订阅缓存的序列号不为空*/
			return i;			
	}
	return SUBSCRIBE_NULL;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:发布缓存 
*********************************************************************************************************
**/
void release_buff_write(release_buff_t* release_buff, data_container_t* data , u32 delay)
{
	for(u8 i = 0 ; i < RELEASE_BUFF_MAX ; i++)
	{
		/*判断缓存是否空*/
		if(release_buff->release[i].serial == 0) /*缓存指针小于最大限制*/
		{
			memcpy(&release_buff->release[i], data, sizeof(data_container_t)); /*复制整个数据块到缓存*/
			release_buff->delay[i] =  delay; /*延时*/
			break;
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:发布数据读取  增加返回值 返回值为延时
*********************************************************************************************************
**/
u16 release_buff_check(release_buff_t* release_buff, business_information_t* data)
{
	u16 return_data=0;
	if(release_buff->pointer > 0) /*判断发布缓存是否不为空*/
	{			
		if(data->release_data.serial == 0) /*判断发布数据是否为空*/
		{
			memcpy(data->release_name, &release_buff->release[release_buff->pointer].name, sizeof(data->release_data.name)); /*更改发布名字*/
			/*复制数据*/
			memcpy(data->release_data.name, &release_buff->release[release_buff->pointer].name, sizeof(data->release_data.name));
			memcpy(data->release_data.buff, &release_buff->release[release_buff->pointer].buff, sizeof(release_buff->release[0].buff));
			data->release_data.length = release_buff->release[release_buff->pointer].length;
			data->release_data.serial = release_buff->release[release_buff->pointer].serial;
			return_data	=	 release_buff->delay[release_buff->pointer];
			/*清零数据 和 指针自减*/
			memset(&release_buff->release[release_buff->pointer], 0, sizeof(data_container_t));
			release_buff->pointer--; 		
		}
	}
	return return_data;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 读取订阅信息 并执行相关回调函数
*********************************************************************************************************
**/
void read_subscribe_payload(business_information_t* business, business_func_t  business_func)
{
	u8 subscribe = SUBSCRIBE_NULL;
	subscribe = check_subscribe_information(business);/*检测订阅信息是否为空*/
	/*判断订阅内部不为空 执行回调函数*/
	if( subscribe != SUBSCRIBE_NULL )
	{	
		(*business_func)(&business->subscribe_data_buff[subscribe]);/*执行回调函数*/
		clear_release_information(&business->subscribe_data_buff[subscribe]);/*清空缓存*/
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:定向发布数据到缓存
*********************************************************************************************************
**/
void directional_release_data_to_cache(release_buff_t* business_release_buff, data_container_t* current_data_container,u8* release_data,u8 release_length, u8* release_name, u8 release_name_length ,u32 dalay)
{
	memcpy(current_data_container->name, release_name, release_name_length);	
	memcpy(current_data_container->buff, release_data, sizeof(current_data_container->buff));
	current_data_container->length = release_length;
	current_data_container->serial = 1;	
	release_buff_write( business_release_buff, current_data_container, dalay);	/*延时*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:定向发布数据
*********************************************************************************************************
**/
void directional_release_data( business_information_t* current_data_container, u8* release_data,u8 release_length, u8* release_name, u8 release_name_length )
{
	/*发布消息 数据*/
	memcpy(current_data_container->release_name, release_name, release_name_length);
	memcpy(current_data_container->release_data.buff, release_data, release_length); 
	/*发布消息 序号*/
	current_data_container->release_data.serial = 1;	
	/*发布消息 数据长度*/
	current_data_container->release_data.length = release_length;	
}
/***********************************************END*****************************************************/

