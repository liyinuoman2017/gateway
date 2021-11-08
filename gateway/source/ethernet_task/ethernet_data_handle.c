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
#include "string.h"
#include "stdlib.h"
#include "debug_bsp.h"
#include "user_type.h"

#include "ethernet_data_handle.h"
#include "ethernet_operation.h"

#include "data_task.h"
///////////////////////////////////////////////网络//////////////////////////////////////////
#include "wizchip_conf.h" //包含需要的头文件
#include "socket.h"       //包含需要的头文件
#include "dhcp.h"         //包含需要的头文件
#include "spi.h"          //包含需要的头文件
#include "ethernet_task.h"
#include "broker_task.h"


#include "data_task.h"

#include "logic_control_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define RECCEIVE_ASC_NUM 100
#define CODE_ADD 5
#define TCP_ANALYSIS_MAX 4
#define SEND_NUM 120


/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef void (*callback_t)(u8* reve_buff,int16_t reve_length);
/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/

business_information_t  ethernet_business=
{
	.release_name = "ethernet",
	.subscribe_name = "logic_ethernet,logic_ethernet_local",		
	.release_data.serial =0,
	.large_data_length = 0,
};

u8 receive_asc_buff[RECCEIVE_ASC_NUM];	
u8  tcp_receive_buff[DATA_BUF_SIZE];    		//数据缓冲区
u8  tcp_receive_set_buff[DATA_BUF_SIZE_4K];    //数据缓冲区

read_data_t uesr_data_updata;

extern system_information_t user_system_data;
u8 send_tcp_buff[5000];

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void cloud_analysis_handle(uint8_t* reve_buff,int16_t reve_length);
void cloud_send_handle(u8 port);
void local_send_handle(void);
void cloud_data_monitor_clear(void);
void tcp_iap_handle(uint8_t* reve_buff,int16_t reve_length);
void id_transformation(u8 *buff, u8 *id_buff);
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 云端服务器连接数据收发处理
*********************************************************************************************************
**/
u8 scoket_cloud(u8 prot)
{
	u16 ret;	
	ret = recv(prot ,tcp_receive_buff, DATA_BUF_SIZE); /*接收来自Client的数据*/ 
	if(ret > 0)                                      
	{
		/*数据处理*/
		cloud_analysis_handle(tcp_receive_buff,ret);
		/*转换字符串 打印输出*/			
		hex_to_ascii(tcp_receive_buff, receive_asc_buff, ret);
		debug_printf("SOCK%d收到数据：%s\r\n", prot, receive_asc_buff);
		/*清数据缓存*/
		memset(receive_asc_buff,0,RECCEIVE_ASC_NUM);				
		memset(tcp_receive_buff,0,DATA_BUF_SIZE);
		/*清空计数*/
		cloud_data_monitor_clear();		
	}
	/*发送数据处理*/
	cloud_send_handle(prot);
	return 0;
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 本地配置服务器数据收发处理
*********************************************************************************************************
**/
u8 scoket_set(u8 prot)
{	
	u16 ret;
	ret = recv(prot, tcp_receive_set_buff, DATA_BUF_SIZE_4K); /*接收来自Client的数据  4K 数据缓存*/ 
	if(ret > 0)                                      
	{
		/*数据处理*/
		tcp_set_handle(tcp_receive_set_buff, ret);
		/*转换部分字符串*/
		if(ret > RECCEIVE_ASC_NUM)	
			ret = RECCEIVE_ASC_NUM;
		/*转换字符串 打印输出*/	
		hex_to_ascii(tcp_receive_set_buff, receive_asc_buff, ret);//
		debug_printf("SOCK%d收到数据：%s\r\n", prot, receive_asc_buff);
		/*清数据缓存*/
		memset(receive_asc_buff, 0, RECCEIVE_ASC_NUM);			
		memset(tcp_receive_set_buff, 0, DATA_BUF_SIZE_4K);						
		
	}
	local_send_handle();	
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 本地配置服务器数据收发处理
*********************************************************************************************************
**/
#define BALANCED_NUM 100
u8 receive_balanced_buff[BALANCED_NUM];
extern u8 scoket_balanced_flag;
extern scoket_information_t user_scoket_balanced;
void close_balanced(u8 cmd);

u8 scoket_balanced(u8 prot)
{	
	u16 ret;
	u8 in_buff[6]; 
	static u16 clk = 0;
	 /*接收来自Client的数据  */ 
	ret = recv(prot, receive_balanced_buff, BALANCED_NUM);	
	if(ret > 0)                                      
	{	
		/*转换部分字符串*/
		if(ret > RECCEIVE_ASC_NUM)	
			ret = RECCEIVE_ASC_NUM;
		/*转换字符串 打印输出*/	
		hex_to_ascii(receive_balanced_buff, receive_asc_buff, ret);//
		debug_printf("SOCK%d收到数据：%s\r\n", prot, receive_asc_buff);
		/*清数据缓存*/
		memset(receive_asc_buff, 0, RECCEIVE_ASC_NUM);
		
		memcpy(&user_scoket_balanced.dest_ip[0] , &receive_balanced_buff[1] , 4);
		user_scoket_balanced.dest_prot =( receive_balanced_buff[5]<< 8 )+receive_balanced_buff[6];

		hex_to_ascii(user_scoket_balanced.dest_ip, receive_asc_buff, 4);//
		debug_printf("收到均衡负载IP：%s 端口号：%d\r\n", receive_asc_buff ,  user_scoket_balanced.dest_prot);
		close_balanced(1);
	}
	if(((clk++)%999) == 0)
	{
		in_buff[0] = 0xaa;
		in_buff[1] = 0xaa;
		/* 转换ID  */
		id_transformation(&in_buff[2], user_system_data.room.ip);
		send(SOCK_BALANCED, in_buff, 6);
	}
	return 0;
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: UDP DNS 域名解析功能
*********************************************************************************************************
**/
u8 scoket_udp(u8 prot)
{
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 向云端服务器接收数据解析
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
void cloud_analysis_handle(uint8_t* reve_buff,int16_t reve_length)
{		
	/*发布消息给LOGIC*/
	directional_release_data(&ethernet_business, reve_buff, reve_length ,(u8*)"ethernet", sizeof("ethernet"));
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 向云端服务器发送心跳包 
*********************************************************************************************************
**/
void cloud_send_handle(u8 port)
{
	u8 subscribe = SUBSCRIBE_NULL;
	/*判断订阅信息是否有数据*/
	subscribe = check_subscribe_information(&ethernet_business);
	if( subscribe != SUBSCRIBE_NULL )
	{
		/*发送数据*/
		if( ( ( strstr("logic_ethernet"  ,(char*)ethernet_business.subscribe_data_buff[subscribe].name ) ) != NULL ) ) 
		{
			send(SOCK_CLOUD, ethernet_business.subscribe_data_buff[subscribe].buff, ethernet_business.subscribe_data_buff[subscribe].length);
			debug_printf("SOCK%d 发送心跳\r\n", port); 
			/*清空信息 */
			clear_release_information(&ethernet_business.subscribe_data_buff[subscribe]);	
		}
		if( ( ( strstr("logic_ethernet_local"  ,(char*)ethernet_business.subscribe_data_buff[subscribe].name ) ) == NULL ) ) 
			/*清空信息 */
			clear_release_information(&ethernet_business.subscribe_data_buff[subscribe]);				
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 向本地服务器发数据 
*********************************************************************************************************
**/
void local_send_handle(void)
{
	u8 subscribe = SUBSCRIBE_NULL;
	/*判断订阅信息是否有数据*/
	subscribe = check_subscribe_information(&ethernet_business);
	if( subscribe != SUBSCRIBE_NULL )
	{
		if( ( ( strstr("logic_ethernet_local"  ,(char*)ethernet_business.subscribe_data_buff[subscribe].name ) ) != NULL ) ) 
		{
			/*打样信息*/
			if( ethernet_business.subscribe_data_buff[subscribe].buff[0] == 0x7b)
			{
				if( ethernet_business.subscribe_data_buff[subscribe].buff[1] == 0x71)
					debug_printf("内部485口上报调试信息\r\n"); 
				else if( ethernet_business.subscribe_data_buff[subscribe].buff[1] == 0x72)
					debug_printf("外部485口上报调试信息\r\n"); 				
			}
			if( ethernet_business.subscribe_data_buff[subscribe].buff[0] == 0x7A)
			{
				if( ethernet_business.subscribe_data_buff[subscribe].buff[1] == 0x71)
					debug_printf("内部485口上报透传信息\r\n"); 
				else if( ethernet_business.subscribe_data_buff[subscribe].buff[1] == 0x72)
					debug_printf("外部485口上报透传信息\r\n"); 				
			}
			/*发送数据*/			
			send(SOCK_LOCAL, ethernet_business.subscribe_data_buff[subscribe].buff, ethernet_business.subscribe_data_buff[subscribe].length);
			/*清空信息 */
			clear_release_information(&ethernet_business.subscribe_data_buff[subscribe]);			
		}
		if( ( ( strstr("logic_ethernet"  ,(char*)ethernet_business.subscribe_data_buff[subscribe].name ) ) == NULL ) )
			/*清空信息 */
			clear_release_information(&ethernet_business.subscribe_data_buff[subscribe]);				
	}		
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 回应配置服务器
*********************************************************************************************************
**/
void tcp_set_reply(uint8_t cmd)
{
	u8 send_buff[10];
	send_buff[0] = 0X7E;
	send_buff[1] = 0;
	send_buff[2] = 1;
	send_buff[3] = cmd;
	send_buff[4] = 0;
	send_buff[5] = 0X7D;
	send(SOCK_LOCAL, send_buff, 6);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 本地服务器配置参数  读取参数 
*********************************************************************************************************
**/
extern u8 firmware_version;
void tcp_set_handle(uint8_t* reve_buff,int16_t reve_length)
{
	/*关键部分不便公开，删除处理*/	
	if(reve_buff[0] == 0x7E)/*配置指令*/
	{
		/*增加 帧头 校验判断 复制数据时减去包头包尾长度类型*/
		if(reve_buff[3] == 0x04)/*场景配置*/
		{
			write_scene_data(&reve_buff[4],reve_length - 5);
			tcp_set_reply(reve_buff[3]);
		}
		else if(reve_buff[3] == 0x03)/*逻辑配置*/
		{	
			write_logic_data(&reve_buff[4],reve_length - 5);
			tcp_set_reply(reve_buff[3]);		
		}
		else if(reve_buff[3] == 0x7B)/* 调试指令开关*/
		{
			/*发布消息给LOGIC*/
			directional_release_data(&ethernet_business, reve_buff, reve_length ,(u8*)"local_ethernet", sizeof("local_ethernet"));
			reve_buff[4] = reve_buff[4]&0x0f;
			memcpy(&send_tcp_buff, reve_buff ,6);
			send(SOCK_LOCAL, send_tcp_buff, 6);	
		}	
	/*关键部分不便公开，删除处理*/		
		
	}
	if((reve_buff[0] == 0XF3) && (reve_length == 44 ))/*F3 调试指令*/
	{		
	/*关键部分不便公开，删除处理*/
	}
	else if( reve_buff[0] == 0X7A)/*0X7A 透传指令*/
	{		
	/*关键部分不便公开，删除处理*/
	}
	/*tcp iap 升级 处理*/
	tcp_iap_handle(reve_buff, reve_length);
}

/***********************************************END*****************************************************/

