/**
*********************************************************************************************************
*                                        
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.01
* 
*********************************************************************************************************
**/
#ifndef __BROKER_BUSINESS_H__
#define __BROKER_BUSINESS_H__

/*
*********************************************************************************************************
Includes 
*********************************************************************************************************
*/


/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
//任务优先级  设置为高优先级
#define BROKER_TASK_PRIO		6
//任务堆栈大小	
#define BROKER_STK_SIZE 		1024  


#define SUBSCRIBE_NULL 			0XFF 
#define RELEASE_BUFF_MAX  50
/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
#define CONTAINER_AMOUNT 150
#define RELEASE_NAME_NUM 30
#define SUBSCRIBE_NAME_NUM 100
typedef struct data_container_def
{
	u8 buff[CONTAINER_AMOUNT + 1];
	u8 serial;	
	u8 name[RELEASE_NAME_NUM + 1];
	u8 length;
}data_container_t;

#define SUBSCRIBE_AMOUNT   10
typedef struct business_information_def
{
	u8 release_name[RELEASE_NAME_NUM + 1];
	u8 subscribe_name[SUBSCRIBE_NAME_NUM +1];	
	u8 business_status;
	data_container_t	release_data;
	data_container_t	subscribe_data_buff[SUBSCRIBE_AMOUNT + 1];	
	u8 large_data_length;
	u8 *large_data_buff;
	u8 large_data_user_length;
}business_information_t;

typedef struct release_buff_def
{
	u8 pointer;
	data_container_t release[RELEASE_BUFF_MAX + 1];
	u32 delay[RELEASE_BUFF_MAX + 1];
}release_buff_t;

typedef void (*business_func_t)(data_container_t* business);

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
void broker_task(void *pvParameters);
void clear_release_information(data_container_t* release);
u8 check_subscribe_information(business_information_t* business);
void read_subscribe_payload(business_information_t* business, business_func_t  business_func);
u16 release_buff_check(release_buff_t* release_buff, business_information_t* data);
void release_buff_write(release_buff_t* release_buff, data_container_t* data , u32 delay);
void directional_release_data_to_cache(release_buff_t* business_release_buff, data_container_t* current_data_container,u8* release_data,u8 release_length, u8* release_name, u8 release_name_length ,u32 dalay);
void directional_release_data( business_information_t* current_data_container, u8* release_data,u8 release_length, u8* release_name, u8 release_name_length );
#endif
/***********************************************END*****************************************************/
