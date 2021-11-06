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
#ifndef __COMMUNICATION_ANALYSIS_H__
#define __COMMUNICATION_ANALYSIS_H__

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
#define JITEJIA_COMMUNICATION   	10
#define DEHE_COMMUNICATION   		11
#define FLAMILY_COMMUNICATION   	12
#define THINK_HOME_COMMUNICATION   	13
#define HOTEL_COMMUNICATION   		14

#define USER_COMMUNICATION 	  	FLAMILY_COMMUNICATION


#define INSTRUCTION_0XF0		0xF0
#define INSTRUCTION_0XF1		0xF1
#define INSTRUCTION_0XF2		0xF2
#define INSTRUCTION_0XF3		0xF3
#define INSTRUCTION_0XF4		0xF4
#define INSTRUCTION_0XF5		0xF5

#define INSTRUCTION_LAMP		0x02
#define INSTRUCTION_WIND		0x10


#define CACHE_BUFF_LONGTH 		5
#define CACHE_BUFF_NUM			200
#define IDLE_NUM				10
/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef struct usart_frame_def
{
	uint8_t	data_cache[CACHE_BUFF_LONGTH][CACHE_BUFF_NUM];     	//通讯缓存
	uint16_t	data_cache_long[CACHE_BUFF_LONGTH]; 				//通讯缓存长度
	uint8_t	data_pointer;										//通讯缓存指针
	uint16_t refresh_num;
	uint16_t current_num;
	uint8_t	receive_buff[CACHE_BUFF_NUM];
	uint8_t 	analysis_buff[CACHE_BUFF_NUM];
	uint16_t analysis_long;
	uint16_t null;
	uint16_t null2;

}usart_frame_t;


typedef  void (*fun_void)(void);

/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/

extern usart_frame_t   usart3_commnuication_frame;
extern usart_frame_t   usart4_commnuication_frame;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void common_receive_refresh(uint8_t receive_data, usart_frame_t *com_pointer);
void common_idle_judge(usart_frame_t *com_pointer);
void common_cache_data(uint16_t buff_long,usart_frame_t *com_pointer);
void data_analysis_handle(usart_frame_t* com_pointer,fun_void  common_analysis_handle);
#endif
/***********************************************END*****************************************************/
