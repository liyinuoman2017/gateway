
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
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define PROGRAM_FIXED_DELAY 5
#define FORMAT_MS 0
#define FORMAT_S 1

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
static data_container_t  current_data_container;
extern system_information_t user_system_data;
extern user_total_information_t  current_user;
extern user_logic_t user_logic;
extern release_buff_t logic_business_release_buff;
extern  u8 key_value_updata_flag;
extern u8 ethernet_debug_flag;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void hotle_agreement_handle(u8* key_buff, data_container_t  *current_data_container);
void inside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container);	
void outside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container);
static void debug_ethernet_transmission(u8* buff, u8 length , u8 port ,u8 cmd);
static void transparent_ethernet_transmission(u8* buff, u8 length );
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议解析
*********************************************************************************************************
**/
static void hotle_agreement_analysis(data_container_t* subscribe)
{
	if((subscribe->buff[0] == 0xf2) && (subscribe->length == 4))
	{
		/*缓存3字节键值*/
		key_buff_write(&subscribe->buff[1],0);	
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:
*********************************************************************************************************
**/
static void inside_lookup_analysis(data_container_t* subscribe)
{
	/*依次查表*/	
	for(u8 i=0; i < MACTH_TABLE_NUM; i++)
	{
		/*判断长度不为0*/
		if((subscribe->length != 0)&&(user_system_data.logic.inside_table[i].length != 0))
		{
			if(compare_hex_buff(subscribe->buff, user_system_data.logic.inside_table[i].buff , 
				(subscribe->length > user_system_data.logic.inside_table[i].length) ? subscribe->length :user_system_data.logic.inside_table[i].length))/* 根据实际长度比较 注意数值溢出*/
			{
				/*缓存3字节键值*/		
				u8 key_buff[3];
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0 ; 
				key_buff[DATA_BYTE] =user_system_data.logic.inside_table[i].value; 
				key_buff_write(key_buff, 1);/*标记键值类型为内部转发*/
				break;
			}
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:
*********************************************************************************************************
**/
static void outside_lookup_analysis(data_container_t* subscribe )
{
	/*依次查表*/
	for(u8 i=0; i < MACTH_TABLE_NUM; i++)
	{
		/*判断长度不为0*/
		if((subscribe->length != 0)&&(user_system_data.logic.outside_table[i].length != 0))
		{
			if(compare_hex_buff(subscribe->buff, user_system_data.logic.outside_table[i].buff , 
				(subscribe->length > user_system_data.logic.outside_table[i].length) ? subscribe->length :user_system_data.logic.outside_table[i].length))/* 根据实际长度比较 注意数值溢出*/		
			{
				/*缓存3字节键值*/		
				u8 key_buff[3];
				key_buff[0] = 0;
				key_buff[STATUS_BYTE] = 0 ; 
				key_buff[DATA_BYTE] =user_system_data.logic.outside_table[i].value; 
				key_buff_write(key_buff, 2);/*标记键值类型为外部转发*/
				break;
			}
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:比较两个HEX  BUFF是否一样
*********************************************************************************************************
**/
u8 compare_hex_buff(u8* buff_s, u8* buff_d , u8 length)
{
	for(u8 i=0; i < length; i++)
		if(buff_s[i] != buff_d[i])
			return 0;
	return 1;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:内部RS485数据解析 
*********************************************************************************************************
**/
static void inside_485_analysis(data_container_t* data_container)
{
	if( ( strstr("rs485inside"  , (char*)data_container->name ) ) != NULL )  /*判断发布名是否为rs485inside*/
	{
		/*酒店协议解析数据 内部需要判断服务*/
		hotle_agreement_analysis( data_container );
		/*查表协议解析数据*/
		inside_lookup_analysis( data_container );
		/*调试数据*/	
		if( 1 == ethernet_debug_flag )	
			debug_ethernet_transmission(data_container->buff , data_container->length , 0x71 , 0x7b);
		/*透传数据数据*/
		if((data_container->buff[0] == 0x7a)&&(data_container->buff[1] == 0x71))
		{
			transparent_ethernet_transmission(data_container->buff , data_container->length );
		}
	}
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	:调试信息
*********************************************************************************************************
**/
static void debug_ethernet_transmission(u8* buff, u8 length , u8 port ,u8 cmd)
{	
	/*透传数据*/	
	memcpy(current_data_container.name, "logic_ethernet_local", sizeof("logic_ethernet_local"));
	current_data_container.buff[0] = cmd;
	current_data_container.buff[1] = port;
	memcpy(&current_data_container.buff[2], buff, length);
	current_data_container.serial = 1;
	current_data_container.length = 2 + length;
	
	release_buff_write( &logic_business_release_buff, &current_data_container, 5); //延时发布数据	
	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:透传信息
*********************************************************************************************************
**/
static void transparent_ethernet_transmission(u8* buff, u8 length)
{	
	/*透传数据*/	
	memcpy(current_data_container.name, "logic_ethernet_local", sizeof("logic_ethernet_local"));

	current_data_container.length = length;
	memcpy(&current_data_container.buff, buff, length);
	current_data_container.serial = 1;	
	
	release_buff_write( &logic_business_release_buff, &current_data_container, 5); //延时发布数据	
	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:外部RS485数据解析
*********************************************************************************************************
**/
static void outside_485_analysis(data_container_t* data_container)
{
	if( ( strstr("rs485outside"  ,(char*)data_container->name ) ) != NULL )  /*判断发布名是否为rs485inside*/
	{
		/*查表协议解析数据*/
		outside_lookup_analysis( data_container );
		/*调试数据*/	
		if( 1 == ethernet_debug_flag )	
			debug_ethernet_transmission(data_container->buff , data_container->length , 0x72, 0x7b);
		if((data_container->buff[0] == 0x7a)&&(data_container->buff[1] == 0x72))
		{
			transparent_ethernet_transmission(data_container->buff , data_container->length);
		}		
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:RS485数据解析
*********************************************************************************************************
**/
void communication_485_analysis(data_container_t* data_container)
{
	/*内部解析数据*/
	inside_485_analysis(data_container);	
	/*外部解析数据*/
	outside_485_analysis(data_container);
		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:通讯数据处理 更新current_user中的数据  
*********************************************************************************************************
**/
void communication_485_handle(u8* key_buff ,u8 type,data_container_t  *current_data_container)
{
	/*外部协议键值解析*/	
	outside_lookup_handle(key_buff , type, current_data_container);	
	/*内部协议键值解析*/
	inside_lookup_handle(key_buff , type, current_data_container);
	/*酒店协议键值解析*/
	hotle_agreement_handle(key_buff, current_data_container);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:40个灯状态 映射到5字节
*********************************************************************************************************
**/
void lamp_byte_mapping(u8 *lamp_byte_buff, u8 *lamp_data_buff)
{
	/*40灯对应5字节*/
	for(u8 i = 0; i < 5; i++)
	{
		/*清零*/
		lamp_byte_buff[ i ] = 0;
		for(u8 j = 0; j < 8 ; j++)
		{
			/*更新8BIT数据*/
			if(lamp_data_buff[i*8 + j] != 0)
				lamp_byte_buff[ i ] |= 0x01<<j;
			else
				lamp_byte_buff[ i ] &= ~(0x01<<j);		
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:16组合 映射到2字节   cmd 0 取反 1开  2 关 
*********************************************************************************************************
**/
void combined_byte_mapping(lamp_status_t* lamp_buff, u8 combined , u8 cmd)
{
	if(cmd == 1) /*置1*/
	{
		if(combined < 8)
			lamp_buff->combination[0] |= 0x01<<	combined;
		else if(combined < 16)
			lamp_buff->combination[1] |= 0x01<<	(combined-8);
	}
	else if(cmd == 2)/*置0*/
	{
		if(combined < 8)
			lamp_buff->combination[0] &= ~(0x01<<combined);
		else if(combined < 16)
			lamp_buff->combination[1] &= ~(0x01<<(combined-8));	
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:灯指令 打包  agreement_version=0 51字节  agreement_version = 1  50字节 
*********************************************************************************************************
**/
void lamp_f1_command_package(u8* package_buff, lamp_status_t* lamp_buff , u16 delay ,u8 second_flag , u8 agreement_version)
{
/*关键部分不便公开，删除处理*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:复制场景模式的40个灯状态 
*********************************************************************************************************
**/
u16 scene_mode_lamp_update_status(lamp_status_t* lamp_buff ,scene_buff_t *mode_buff)
{
	for(u8 i = 0; i < 40 ; i ++)
		if( mode_buff->data[ 11 + i ] != 0xff) /*不复制nc状态*/
			lamp_buff->lamp_buff[ i ] =	 mode_buff->data[ 11 + i ];
	/*延时*/				
	return ((u16)mode_buff->data[ 9 ]*256 + mode_buff->data[ 10 ]);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:根据操作模式控制组合灯状态 
*********************************************************************************************************
**/
u16 combination_lamp_update_status(lamp_status_t* lamp_buff ,scene_buff_t *mode_buff , u8 cmd)
{
	/*状态为1 按照组合数据执行，状态为0 按照组合数据取反执行 */
	if(cmd == 1) /*复制组合数据*/
	{
		for(u8 i = 0; i < 40 ; i ++)
			if( mode_buff->data[ 11 + i ] != 0xff) /*不处理nc状态*/
				lamp_buff->lamp_buff[ i ] =	 mode_buff->data[ 11 + i ];
	}	
	/*延时*/				
	return ((u16)mode_buff->data[ 9 ]*256 + mode_buff->data[ 10 ]);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:同步组合灯状态
*********************************************************************************************************
**/
void combination_state_synchronization(u8* lamp_buff , u8 * combination_state)
{
	u8 identical_flag, contrary_flag;
	/* 与组合数据一致，状态为1，与组合数据取反一致，状态为0 ，其他情况不改变状态 */	
	for(u8 i = 0 ; i < 16 ; i++)
	{
		/*清状态位*/
		identical_flag =  0;
		contrary_flag  =  0;
		/*判断40个灯状态 网络只配置了38个数据*/
		for(u8 j = 0 ; j < 38 ; j++)
		{
			if( user_system_data.scene.scene_data[16 + i].data[11 + j] != 0xff) /*不处理nc状态*/
			{
				/*比较场景值是否和灯状态相同*/
				if( (!user_system_data.scene.scene_data[16 + i].data[11 + j]) != (!lamp_buff[j]) )
					identical_flag = 1;
				else
					contrary_flag = 1;			
			}
		}		
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:缓存发布数据 
*********************************************************************************************************
**/
void rs485inside_release_data_to_cache(u8* buff, u8 num ,data_container_t  *current_data_container , u32 delay)
{
	directional_release_data_to_cache( &logic_business_release_buff ,
		current_data_container, 
		buff , 
		num , 
		(u8*)"logic_rs485inside", 
		sizeof("logic_rs485inside"),
		delay);  /*内部酒店协议发送固定延时*/
	/*调试数据*/	
	if( 1 == ethernet_debug_flag )		
		debug_ethernet_transmission(buff , num, 0x71, 0x7b);			
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:服务数据转换 
*********************************************************************************************************
**/
const u8 service_maping[8] ={0,4,3,6,2,7,1,5};
void service_transformation(u8* key_buff)
{
	u8 *service_pointer = &current_user.service.card_power_on_status;
	if(key_buff[STATUS_BYTE] == 1) /*置1*/
	{
		service_pointer[ service_maping[ key_buff[DATA_BYTE] - 20 ] ] = 1;		
	}
	else if(key_buff[STATUS_BYTE]  == 2)/*置0*/
	{
		service_pointer[ service_maping[ key_buff[DATA_BYTE] - 20 ] ] = 0;
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:服务指令 打包 
*********************************************************************************************************
**/
void service_f0_command_package(u8* package_buff, service_status_t* service_buff )
{
/*关键部分不便公开，删除处理*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:音乐
*********************************************************************************************************
**/
static void hotle_agreement_music(u8* key_buff)
{	
/*关键部分不便公开，删除处理*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议灯处理    
*********************************************************************************************************
**/
static void hotle_agreement_lamp_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*灯更新总状态*/
	if(key_buff[STATUS_BYTE] == 0)/*灯取反*/
		current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] = !	current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START];
	/*灯开为100 兼容调光灯*/
	if(current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] == 1)
		current_user.lamp.lamp_buff[ key_buff[DATA_BYTE] - KEY_VALUE_LAMP_START] = 100;
	/*灯指令打包  统一使用场景一的延时*/ 
	lamp_delay = user_system_data.scene.scene_data[0].data[9]*256 + user_system_data.scene.scene_data[0].data[10];
	lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp , lamp_delay , FORMAT_MS , user_system_data.set.agreement_version );	
	/*发布整包数据  帧头 场景 延时 灯状态 */
	rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50 , current_data_container, PROGRAM_FIXED_DELAY);//延时发布数据 程序延时
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议场景处理  
*********************************************************************************************************
**/
static void hotle_agreement_scene_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*场景更新总状态  自恢复和绝对开执行*/
	if( (key_buff[STATUS_BYTE] == 0) || (key_buff[STATUS_BYTE] == 1) )
	{
		current_user.lamp.mode = key_buff[DATA_BYTE] - KEY_VALUE_SECEN_START;
		/*更新灯状态*/
		lamp_delay = scene_mode_lamp_update_status(&current_user.lamp, &user_system_data.scene.scene_data[ current_user.lamp.mode ] );
		/*灯指令打包*/
		lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp , lamp_delay , FORMAT_MS, user_system_data.set.agreement_version );	
		/*发布整包数据  帧头 场景 延时 灯状态*/	
		rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50, current_data_container , PROGRAM_FIXED_DELAY); //延时发布数据 程序延时
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议组合处理   
*********************************************************************************************************
**/
static u8 combination_state_buff[16];
static void hotle_agreement_combination_handle(u8* key_buff, data_container_t  *current_data_container)
{

	u8 instruction_packaging_buff[60];
	u16 lamp_delay;	
	/*组合更新总状态  自恢复取反  和  高低有效绝对开关执行*/
	if( (key_buff[STATUS_BYTE] == 0) || (key_buff[STATUS_BYTE] == 1) || (key_buff[STATUS_BYTE] == 2) )
	{
		u8 combination_num;
		combination_num = key_buff[DATA_BYTE]-KEY_VALUE_COMBINATION_START;
		combined_byte_mapping(&current_user.lamp, combination_num ,key_buff[STATUS_BYTE]);
		if(key_buff[STATUS_BYTE] == 0)
			combination_state_buff[combination_num] = !combination_state_buff[combination_num];			
		/*更新灯状态*/
		lamp_delay =combination_lamp_update_status(&current_user.lamp, &user_system_data.scene.scene_data[ key_buff[DATA_BYTE] - KEY_VALUE_SECEN_START] ,combination_state_buff[combination_num]);
		/*灯指令打包*/
		lamp_f1_command_package(instruction_packaging_buff, &current_user.lamp, lamp_delay , FORMAT_MS, user_system_data.set.agreement_version );		
		/*发布整包数据  帧头 场景 延时 灯状态 */
		rs485inside_release_data_to_cache(instruction_packaging_buff, (user_system_data.set.agreement_version == 0) ? 51 : 50, current_data_container , PROGRAM_FIXED_DELAY); //延时发布数据
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议服务处理     
*********************************************************************************************************
**/
static void hotle_agreement_service_handle(u8* key_buff, data_container_t  *current_data_container)
{
	u8 instruction_packaging_buff[60];	
	/*服务数据转换并更新信息*/
	service_transformation(key_buff);
	/*打包布服务指令*/
	service_f0_command_package(instruction_packaging_buff,&current_user.service);
	/*发布服务状态 9字节*/
	rs485inside_release_data_to_cache(instruction_packaging_buff, 9, current_data_container , 50);	//延时发布数据 500ms延时 增加服务指令反馈延时，扩大插卡指令和服务指令的时间差
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:酒店协议处理：灯，场景，组合，服务。   key_buff[STATUS_BIT]为状态    key_buff[DATA_BYTE]为键值 
*********************************************************************************************************
**/
static void hotle_agreement_handle(u8* key_buff, data_container_t  *current_data_container)
{
	if((key_buff[DATA_BYTE] >= KEY_VALUE_LAMP_START) && (key_buff[DATA_BYTE] <= KEY_VALUE_LAMP_END))/*灯操作*/
	{	
		hotle_agreement_lamp_handle(key_buff, current_data_container);
	}
	else if((key_buff[DATA_BYTE] >= KEY_VALUE_SECEN_START) && (key_buff[DATA_BYTE] <= KEY_VALUE_SECEN_END))/*场景 模式 192~207  */
	{
		hotle_agreement_scene_handle(key_buff, current_data_container);
	}
	else if(key_buff[DATA_BYTE] == 30)/* 开锁*/
	{
		if(key_buff[STATUS_BYTE] == 1 ) 
			current_user.service.door_lock_status = 1;
		else if(key_buff[STATUS_BYTE] == 2 )
			current_user.service.door_lock_status = 0;
	}
	else if(( key_buff[DATA_BYTE] >= 170 ) && (key_buff[DATA_BYTE] <= 177))/* 音乐指令*/
	{
		hotle_agreement_music( key_buff );
	}	

	/* 同步组合状态 */
	combination_state_synchronization( current_user.lamp.lamp_buff , combination_state_buff);	
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	:内部485查表处理
*********************************************************************************************************
**/
void inside_lookup_handle(u8* key_buff,u8 type , data_container_t  *current_data_container)
{
	if(type != 1)
	{
		/*依次查表*/
		for(u8 i=0; i < MACTH_TABLE_NUM; i++)
		{
			/*匹配键值 不能为绝对关*/			
			if( (key_buff[DATA_BYTE] == user_system_data.logic.inside_table[i].value ) && (key_buff[STATUS_BYTE] != 2 ) )
			{

				if(user_system_data.logic.inside_table[i].length != 0)
				{
					/*根据类型选择输出通道*/
					u8 name_buff[20];
					u8 name_length;
					if(user_system_data.logic.inside_table[i].type == 0)
					{					
						name_length = sizeof("logic_rs485inside");
						memcpy(name_buff , "logic_rs485inside" ,sizeof("logic_rs485inside"));
						/*调试信息上报*/	
						if( 1 == ethernet_debug_flag )	
							debug_ethernet_transmission(user_system_data.logic.inside_table[i].buff , user_system_data.logic.inside_table[i].length , 0x71, 0x7b);
						
					}				
					else
					{
						name_length = sizeof("logic_rs485all");
						memcpy(name_buff , "logic_rs485all" ,sizeof("logic_rs485all"));
						/*调试信息上报*/
						if( 1 == ethernet_debug_flag )	
							debug_ethernet_transmission(user_system_data.logic.inside_table[i].buff , user_system_data.logic.inside_table[i].length , 0x73, 0x7b);							
					}
					/*定向发布消息*/
					directional_release_data_to_cache( 
					&logic_business_release_buff ,
					current_data_container, 
					user_system_data.logic.inside_table[i].buff , 
					user_system_data.logic.inside_table[i].length , 
					name_buff, 
					name_length,
					(user_system_data.logic.inside_table[i].delay[0]*256 + user_system_data.logic.inside_table[i].delay[1])/10);	//延时发布数据   ms
					
				}				
					
			}
		}
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:外部485查表处理
*********************************************************************************************************
**/
void outside_lookup_handle(u8* key_buff,u8 type, data_container_t  *current_data_container)
{
	if(type != 2)
	{
		/*依次查表*/		
		for(u8 i=0; i < MACTH_TABLE_NUM; i++)
		{	
			/*匹配键值*/			
			if( (key_buff[DATA_BYTE] == user_system_data.logic.outside_table[i].value) && (key_buff[STATUS_BYTE] != 2 ) )
			{
				if(user_system_data.logic.outside_table[i].length != 0)
				{
					/*根据类型选择输出通道*/
					u8 name_buff[20];
					u8 name_length;
					if(user_system_data.logic.outside_table[i].type == 0)
					{					

						name_length = sizeof("logic_rs485inside");
						memcpy(name_buff , "logic_rs485inside" ,sizeof("logic_rs485inside"));
						
						/*调试信息上报*/
						if( 1 == ethernet_debug_flag )
							debug_ethernet_transmission(user_system_data.logic.outside_table[i].buff , user_system_data.logic.outside_table[i].length , 0x71, 0x7b);	
					}				
					else
					{
						name_length = sizeof("logic_rs485all");
						memcpy(name_buff , "logic_rs485all" ,sizeof("logic_rs485all"));
						/*调试信息上报*/
						if( 1 == ethernet_debug_flag )
							debug_ethernet_transmission(user_system_data.logic.outside_table[i].buff , user_system_data.logic.outside_table[i].length , 0x73, 0x7b);							
					}
					/*定向发布消息*/
					directional_release_data_to_cache( 
					&logic_business_release_buff ,
					current_data_container, 
					user_system_data.logic.outside_table[i].buff , 
					user_system_data.logic.outside_table[i].length , 
					name_buff, 
					name_length,
					(user_system_data.logic.outside_table[i].delay[0]*256 + user_system_data.logic.outside_table[i].delay[1])/10);	//延时发布数据	 ms		
				}
			}
		}
	}
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	:
*********************************************************************************************************
**/
void hotel_air_conditioner_package_send(air_conditioner_t *air_conditioner )
{
/*关键部分不便公开，删除处理*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:
*********************************************************************************************************
**/
void analysis_of_air_conditioning_instructions(data_container_t* data_container)
{
	if( ( strstr("rs485inside"  , (char*)data_container->name ) ) != NULL )  /*判断发布名是否为rs485inside*/
	{
		/*判断空调指令*/
		if((data_container->buff[0] == 0xf4) && (data_container->buff[1] == 0xf3) )
		{
			/* 2个空调 */
			if(data_container->buff[2] < 2)
			{
				u8 add = data_container->buff[2];
				
				current_user.air_conditioner[add].set_temperature = data_container->buff[3] ;
				current_user.air_conditioner[add].wind_speed = data_container->buff[4] ;
				current_user.air_conditioner[add].wind_direction = data_container->buff[5] ;
				current_user.air_conditioner[add].power = data_container->buff[6] ;
				current_user.air_conditioner[add].mode = data_container->buff[8] ;
				current_user.air_conditioner[add].real_temperature = data_container->buff[9] ;
				
				key_value_updata_flag	= 1;			
			}
		}
	}
}
/***********************************************END*****************************************************/

