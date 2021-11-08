
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
extern key_value_frame_t user_key_data;
extern key_value_frame_t user_key_analysis;
extern key_value_frame_t user_key_execute;
extern system_information_t user_system_data;
extern key_value_delay_frame_t  user_delay_key;
extern user_logic_t user_logic;
extern user_total_information_t  current_user;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空用户键值缓存
*********************************************************************************************************
**/
void key_buff_clear(void)
{
	memset(&user_key_data, 0, sizeof( key_value_frame_t ));
	user_key_data.key_opint=0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值定向缓存
*********************************************************************************************************
**/
void key_write_directional_cache(u8 *buff , u8 type , key_value_frame_t *target_cache)
{
	if(target_cache->key_opint < KEY_VALUE_NUM)
	{
		/*从1开始存储*/
		target_cache->key_opint++;
		/*数据缓存*/
		memcpy(&target_cache->keyvalue[target_cache->key_opint], buff, sizeof(target_cache->keyvalue[0].buff));
		target_cache->keyvalue[target_cache->key_opint].type = type;  /*  类型为标记 内部外部转发解析  */
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:用户键值解析写入
*********************************************************************************************************
**/
void key_buff_write(u8* buff,u8 type)
{	
	key_write_directional_cache(buff , type , &user_key_analysis);
	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值定向读出
*********************************************************************************************************
**/
u8 key_read_directional_cache(u8 *buff , key_value_frame_t *target_cache)
{
	u8 ret_data = 0;
	if(target_cache->key_opint > 0)
	{
		/*复制缓存*/
		memcpy(buff, &target_cache->keyvalue[target_cache->key_opint],sizeof(target_cache->keyvalue[0].buff));
		ret_data = target_cache->keyvalue[target_cache->key_opint].type;
		/*清空缓存*/
		memset(&target_cache->keyvalue[target_cache->key_opint], 0, sizeof(target_cache->keyvalue[0].buff));
		target_cache->key_opint--;		
	}
	return ret_data;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:用户键值读出执行
*********************************************************************************************************
**/
u8 key_buff_read(u8* buff)
{
	u8 ret_data = 0;
	ret_data = key_read_directional_cache( buff ,&user_key_execute);
	return ret_data;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值缓存
*********************************************************************************************************
**/
void key_delay_buff_write(u8* buff, u8 type, u32 delay)
{
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag == 0)
		{
			memcpy(&user_delay_key.keyvalue[i], buff, sizeof(user_delay_key.keyvalue[0].buff));
			user_delay_key.keyvalue[i].type = type;  /*  类型为标记 内部外部转发解析  */
			user_delay_key.keyvalue[i].delay = delay;
			user_delay_key.keyvalue[i].flag = 1;
			break;
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值缓存
*********************************************************************************************************
**/
void key_delay_buff_directional_clear(u8 value)
{
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag != 0)  
		{
			if(user_delay_key.keyvalue[i].buff[2] == value)
			{
				/*清空所有缓存数据*/
				memset( &user_delay_key.keyvalue[i], 0, sizeof(key_value_delay_t) );
			}				
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值扫描，输出到执行缓存
*********************************************************************************************************
**/
void key_delay_scanf_handle(void)
{
	/*10MS执行一次*/
	for(u8 i = 0 ; i <KEY_VALUE_NUM ; i++)
	{
		if(user_delay_key.keyvalue[i].flag != 0)
		{
			if(user_delay_key.keyvalue[i].delay > 0)
			{
				user_delay_key.keyvalue[i].delay--;	
			}
			else
			{
				key_write_directional_cache(user_delay_key.keyvalue[i].buff , user_delay_key.keyvalue[i].type , &user_key_execute);
				/*清空所有缓存数据*/
				memset( &user_delay_key.keyvalue[i], 0, sizeof(key_value_delay_t) );					
			}
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:关掉所有设备 上电是可以使用 
*********************************************************************************************************
**/
void key_card_power_off(void)
{
	u8 key_buff[5];
	/*插卡*/
	key_buff[STATUS_BYTE] =  0;
	key_buff[DATA_BYTE] = 193; 
	key_write_directional_cache(key_buff, 0, &user_key_execute);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值缓存定向判断
*********************************************************************************************************
**/
u8 key_directional_check(key_value_frame_t *target_cache)
{	
	if(target_cache->key_opint > 0)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值缓存判断 
*********************************************************************************************************
**/
u8 key_buff_check(void)
{	
	if(key_directional_check(&user_key_execute) == 1)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:插卡判断
*********************************************************************************************************
**/
void key_card_power_on_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	/*插卡判断并关联场景1 2*/
	if(key_logic->buff[DATA_BYTE] == 0x14)
	{
		if(key_logic->buff[STATUS_BYTE] == 1)
		{
			/*插卡*/
			key_buff[STATUS_BYTE] =  0;
			key_buff[DATA_BYTE] = 192; 
			key_write_directional_cache(key_buff, 0, &user_key_execute);
			/*清除拔卡延时*/
			key_delay_buff_directional_clear(193);
			user_logic.power_on_card = 1;
		}		
		else if(key_logic->buff[STATUS_BYTE] == 2)
		{
				/*拔卡*/
			key_buff[STATUS_BYTE] =  0;			
			key_buff[DATA_BYTE] = 193;
			
			if(user_system_data.set.pick_up_card_delay[1] > 60)
				key_delay_buff_write(key_buff, 0, 6000);
			else
				key_delay_buff_write(key_buff, 0, user_system_data.set.pick_up_card_delay[1]*100);	
			
			user_logic.power_on_card = 0;
		}
	}	
	/*  判断映射控制中的 不受插卡限制的指令*/	
	if(user_logic.power_on_card == 0)
	{
		if(key_logic->buff[DATA_BYTE] != 0x14)
		{
			u8 limit = 0;
			for(u8 i = 0 ; i < 10 ; i++)
			{
				if(user_system_data.set.user_mapping_set[i].limit == 1)
				{
					if(user_system_data.set.user_mapping_set[i].function ==	 key_logic->buff[DATA_BYTE])
					{
						limit = 1;
						break;
					}			
				}		
			}
			if(limit == 0)
				key_logic->buff[DATA_BYTE]	= 0xff; 
		}			
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:互斥判断  
*********************************************************************************************************
**/
void key_mutex_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	/*勿扰状态无法清零*/
	if(current_user.service.do_not_disturb_status == 1)
	{
		if(key_logic->buff[DATA_BYTE] == 22)
		{
			key_logic->buff[DATA_BYTE]	= 0xff; 
		}
	}
	/*勿扰键值清除清零状态*/	
	if(key_logic->buff[DATA_BYTE] == 24)
	{

		if(current_user.service.cleaning_status == 1)	
		{
			current_user.service.cleaning_status = 0;
			key_buff[STATUS_BYTE] =  2;				
			key_buff[DATA_BYTE] = 22;
			key_write_directional_cache(key_buff, 0, &user_key_execute);
		}
	}	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:映射判断  原键值不变  增加一个灯控键值和 延时控制键值  
*********************************************************************************************************
**/
void key_mapping_judge(key_value_t *key_logic)
{
	u8 key_buff[5];
	for(u8 i = 0 ; i < 10 ; i++)
	{
		if(user_system_data.set.user_mapping_set[i].channel <= 40)
		{
			/*匹配键值*/
			if((user_system_data.set.user_mapping_set[i].function == key_logic->buff[DATA_BYTE])  && (key_logic->buff[DATA_BYTE] != 255))
			{
				/*判断类型*/
				if(user_system_data.set.user_mapping_set[i].trigger == 0) /*普通类型*/
				{
					/*直接延时发送*/
					key_buff[STATUS_BYTE] =  0;			
					key_buff[DATA_BYTE] = KEY_VALUE_LAMP_START - 1 + user_system_data.set.user_mapping_set[i].channel;	
					key_delay_buff_write(key_buff, 0, ((u16)(user_system_data.set.user_mapping_set[i].delay[0]<<8) + user_system_data.set.user_mapping_set[i].delay[1])*100);				
				}	
				else if(user_system_data.set.user_mapping_set[i].trigger == 2)/*延时触发类型   延时一段时间后开*/
				{
					/*延时发送开*/
					key_buff[STATUS_BYTE] =  1;			
					key_buff[DATA_BYTE] = KEY_VALUE_LAMP_START- 1 + user_system_data.set.user_mapping_set[i].channel;	
					key_delay_buff_write(key_buff, 0, ((u16)(user_system_data.set.user_mapping_set[i].delay[0]<<8) + user_system_data.set.user_mapping_set[i].delay[1])*100);					
				}				
				break;
			}			
		}		
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:键值逻辑处理：插卡判断，互斥判断，断映射判断
*********************************************************************************************************
**/
void key_value_logic_handle(void)
{
	key_value_t  key_logic;
	if(key_directional_check(&user_key_analysis) == 1)
	{
		/*读取键值*/
		key_logic.type = key_read_directional_cache(key_logic.buff ,&user_key_analysis);	
		/*插卡判断*/
		key_card_power_on_judge(&key_logic);
		/*互斥判断*/
		key_mutex_judge(&key_logic);		
		/*映射判断*/
		key_mapping_judge(&key_logic);
		/*写入缓存*/	
		if(key_logic.buff[DATA_BYTE]	!= 0xff)
			key_write_directional_cache(key_logic.buff , key_logic.type , &user_key_execute);
	}
}
/***********************************************END*****************************************************/

