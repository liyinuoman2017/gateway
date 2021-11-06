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
#ifndef __LOGIC_BUSINESS_H__
#define __LOGIC_BUSINESS_H__

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
#define LOGIC_TASK_PRIO		6
//任务堆栈大小	
#define LOGIC_STK_SIZE 		2024  


#define KEY_VALUE_NUM 100



#define  STATUS_BYTE 1
#define  DATA_BYTE 	2


#define KEY_VALUE_LAMP_START   120
#define KEY_VALUE_LAMP_END     159
#define KEY_VALUE_SECEN_START   192
#define KEY_VALUE_SECEN_END     207
#define KEY_VALUE_COMBINATION_START   208
#define KEY_VALUE_COMBINATION_END     223
#define KEY_VALUE_SERVICE_START   20
#define KEY_VALUE_SERVICE_END     27


/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef struct key_value_def
{
	u8 buff[3];
	u8 type;
}key_value_t;

typedef struct key_value_frame_def
{
	key_value_t  keyvalue[KEY_VALUE_NUM+1];
	u8 key_opint;
}key_value_frame_t;

typedef struct key_value_delay_def
{
	u8 buff[3];
	u8 type;
	u32 delay;
	u8 flag;
}key_value_delay_t;

typedef struct key_value_delay_frame_def
{
	key_value_delay_t  keyvalue[KEY_VALUE_NUM+1];
	u8 key_opint;
}key_value_delay_frame_t;

#define MACTH_TABLE_NUM 48
typedef struct  match_data_def
{
	u8 key[3];
	u8 type;
	u16 delay;
	u8  data[12];
	
}match_data_t;
/*匹配缓存数据*/
typedef struct  match_table_def
{
	match_data_t buff[MACTH_TABLE_NUM];
}match_table_t;
/*按键缓存数据*/
typedef struct contac_data_def
{
	u8 key[3];
}contac_data_t;
/*房间数据*/
typedef struct room_information_def
{
	u8 ip[6];
	u8 null[94];
}room_information_t;
/*映射控制数据*/
typedef struct mapping_set_def
{	    
	u8 function;
	u8 channel;
	u8 delay[2];
	u8 trigger;
	u8 limit;
}mapping_set_t;
/*系统设置数据*/
typedef struct set_information_def
{
	u8 central_air_conditioning;
	u8 strong_current_window_screen;
	u8 sequential_time;
	u8 infrared_air_conditioner[2];	
	u8 somatosensory_channel;
	u8 somatosensory_channel_delay[2];
	u8 gate_magnetic_channel;
	
	u8 gate_magnetic_channel_delay[2];
	u8 pick_up_card_delay[2];
	u8 doorbell[2];
	u8 exhaust_passage;
	u8 cloud_selection;
	
	u8 domain_name[30];
	u8 domain_name_length;
	u8 domain_name_port[2];
	u8 upload_time_in_the_cloud[2];
	
	u8 load_balancing_ip[4];
	u8 load_balancing_port[2];
	u8 final_server_ip[4];
	u8 final_serve_port[2];		
	u8 agreement_version;
	mapping_set_t  user_mapping_set[10];
	u8 null[34];
	
}set_information_t;
/*IO输入数据 2字节*/
typedef struct input_data_def
{
	u8 value;
	u8 type;
}input_data_t;
/*转发数据  25字节*/
typedef struct retransmission_data_def
{
	u8 value;
	u8 buff[20];
	u8 length;
	u8 delay[2];
	u8 type;
}retransmission_data_t;

/*逻辑控制数据 16+32*2+48*25*2 = 2480字节*/
typedef struct logic_information_def
{
	u8 output[16];
	input_data_t input[32];
	retransmission_data_t inside_table[48];
	retransmission_data_t outside_table[48];
}logic_information_t;

/*场景数据*/
typedef struct scene_buff_def
{
	u8 data[51];
}scene_buff_t;

/*32组51字节场景数据 1632 字节  16组场景 16组组合 1组灯光亮度限制*/
typedef struct scene_information_def
{
	scene_buff_t scene_data[33];
}scene_information_t;


/* 场景1632 字节  逻辑2480字节  房间 100字节  系统100字节   4312  */
typedef struct system_information_def
{
	room_information_t  room;
	set_information_t   set;
	logic_information_t logic;   
	scene_information_t scene;
	u8 check[10];
	u8 null[190];
}system_information_t;


/**************************************** 逻辑总信息 ****************************************/

typedef struct id_information_def
{
/*	u8 	floor;
	u8  layer;
	u8  romm;
	u8  spare;
*/
	u8 	id[4];
}id_information_t;

typedef struct service_status_def
{
	u8 	card_power_on_status;
	u8 	doorbell_status;
	u8 	do_not_disturb_status;
	u8 	cleaning_status;
	u8 	sos_status;
	u8 	call_service_status;
	u8 	reservation_check_status;
	u8 	please_check_status_late;
	u8 	gate_magnetic_state;
	u8 	wardrobe_1_Magnetic_status;
	u8 	wardrobe_2_magnetic_status;
	u8 	safe_status;
	u8 	door_lock_status;
	u8 	infrared_1_Status;
	u8 	infrared_2_status;
	u8  nobody_status;
	u8  service_reservation1;
	u8  service_reservation2;
	u8  service_reservation3;
	u8  service_reservation4;
}service_status_t;

typedef struct air_conditioner_def
{
	u8 set_temperature;
	u8 wind_speed;
	u8 wind_direction;
	u8 power;
	u8 null;
	u8 mode;
	u8 real_temperature;
	u8 null2;
}air_conditioner_t;

typedef struct music_def
{
	u8 power;
	u8 volume;
	u8 channel;	
	u8 indexes;
	u8 null1;
	u8 null2;
	u8 null3;
	u8 null4;	
}music_t;

typedef struct curtains_def
{
	u8 curtains;
	u8 gauze;
	
}curtains_t;

typedef struct lamp_status_def
{
	u8 mode;
	u8 null1;
	u8 null2;
	u8 null3;
	u8 combination[4];
	u8 lamp_buff[40];
	
}lamp_status_t;

typedef struct environmental_information_def
{
	u8 temperature;
	u8 co2;
	u8 formaldehyde;
	u8 pm25;
	u8 humidity;
	u8 null1;
	u8 null2;
	u8 null3;
	u8 electricity_meter[2]; 
	u8 water_mete[2];
	
}environmental_information_t;


typedef struct user_total_information_def
{
	id_information_t  id;
	service_status_t  service;
	air_conditioner_t  air_conditioner[2];
	music_t			music;	
	curtains_t    	curtains[4];
	lamp_status_t 	lamp;
	environmental_information_t  environmental;
}user_total_information_t;

typedef struct user_logic_def
{
	u8 power_on_card;
}user_logic_t;
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
void logic_control_task(void *pvParameters);
static void key_value_analysis_cache(void);
static void key_value_execute(void);


static void output_control_handle(u8* key_buff);


void key_buff_clear(void);
void key_buff_write(u8* buff, u8 type);
u8 key_buff_read(u8* buff);
u8 key_buff_check(void);
u8 compare_hex_buff(u8* buff_s, u8* buff_d , u8 length);

void key_delay_buff_write(u8* buff, u8 type, u32 delay);
u8 key_delay_buff_read(u8* buff);
void key_delay_scanf_handle(void);
void key_delay_buff_directional_clear(u8 value);
#endif
/***********************************************END*****************************************************/
