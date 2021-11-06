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
#include "ethernet_data_handle.h"
#include "ethernet_operation.h"

#include "string.h"
#include "stdlib.h"
#include "debug_bsp.h"
#include "user_type.h"

///////////////////////////////////////////////网络//////////////////////////////////////////
#include "wizchip_conf.h" //包含需要的头文件
#include "socket.h"       //包含需要的头文件
#include "dhcp.h"         //包含需要的头文件
#include "spi.h"          //包含需要的头文件
#include "dns.h"

#include "logic_control_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/

#define  IPPORT_DOMAIN	53

/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
typedef struct scoket_prot_handle_def
{
	u8 port;
	callback_u8_u8 handle;
	u8 close_flag;	
}scoket_prot_handle_t;
/*
*********************************************************************************************************
Variables
*********************************************************************************************************
*/
u16 ethernet_data_monitor_num = 0;
u8 	dns_server_ip[4] = {114,114,114,114};

u8 domain_name_buff[50];			
u8* domain_name = domain_name_buff;
u8 dns_finish_flag = 0;
extern system_information_t user_system_data;
char my_dhcp_retry = 0;         //DHCP当前共计重试的次数
/*MAC地址，不要和路由器下其他的设备一样 最后一位要为偶数*/
wiz_NetInfo gWIZNETINFO =       
{        
	0x08, 0x08, 0xdc,0x00, 0xab, 0xc8, 
}; 
/*---------------------------------------------------------------*/
/*         服务器IP地址和端口号，根据自己的情况修改              */
/*---------------------------------------------------------------*/

/*远程无服务端口*/
scoket_information_t user_scoket_cloud=
{	
	.dest_ip[0] = 106,
	.dest_ip[1] = 14,
	.dest_ip[2] = 201,
	.dest_ip[3] = 191,	
	.dest_prot = 5188,	
};
/*配置端口 IP 自动获取*/
scoket_information_t user_scoket_set=
{
	.dest_ip[0] = 0,
	.dest_ip[1] = 0,
	.dest_ip[2] = 0,
	.dest_ip[3] = 208,
	.dest_prot = 5188,
};

/*  均衡IP */
scoket_information_t user_scoket_balanced;


scoket_information_t user_scoket_prot_null=
{
	.dest_ip[0] = 192,
	.dest_ip[1] = 168,
	.dest_ip[2] = 1,
	.dest_ip[3] = 1,
	.dest_prot = 5188,
};
scoket_information_t *user_scoket_point_buff[8]=
{
	&user_scoket_prot_null,
	&user_scoket_cloud,	
	&user_scoket_prot_null,
	&user_scoket_set,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
	&user_scoket_prot_null,
};

u8  dhcp_receive_buff[DATA_BUF_SIZE];    //数据缓冲区

static u8 scoket_connect_flag[8]={0,0,0,0,0,0,0,0}; 	/* 8个socket 重启计数器*/	
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
u8 scoket_tcp_handle(u8 user_port ,callback_u8_u8 scoket_fuc);
u8 scoket_dns_handle(u8 user_port ,callback_u8_u8 scoket_fuc);
u8 scoket_cloud(u8 prot);
u8 scoket_set(u8 prot);
u8 scoket_udp(u8 prot);
u8 scoket_balanced(u8 prot);
scoket_prot_handle_t user_scoket_prot_handle[4]=
{
	{
		.port = SOCK_CLOUD,
		.handle = scoket_cloud,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_LOCAL,
		.handle = scoket_set,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_DNS,
		.handle = scoket_udp,
		.close_flag = 0 ,
	},
	{
		.port = SOCK_BALANCED,
		.handle = scoket_balanced,
		.close_flag = 0 ,
	}
	
};

void w5500_reset(void);
void w5500_set(void);
void cloud_data_monitor_reset(void);
void clear_connect_num(void);
/**
*********************************************************************************************************
* @名称	: 
* @描述	:  
*********************************************************************************************************
**/
void w5500_user_reset(void)
{
	w5500_reset();
	vTaskDelay(500);
	w5500_set();
	vTaskDelay(500);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 读取UID  写入MAC
*********************************************************************************************************
**/
void read_uid_num(void)
{
	u32 num;
	num = M4_EFM->UQID3;
	gWIZNETINFO.mac[0] = num>>24;
	gWIZNETINFO.mac[1] = num>>16;
	gWIZNETINFO.mac[2] = num>>8;
	gWIZNETINFO.mac[3] = num;	
	
	num = M4_EFM->UQID2;
	gWIZNETINFO.mac[4] = num>>8;
	gWIZNETINFO.mac[5] = ((num>>8)&0xf0)|0x04;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:  
*********************************************************************************************************
**/
void ethernet_driver_init(void)
{	
	/*读取UID 写入MAC*/
	read_uid_num();
	/*复位W5500*/
	w5500_user_reset();	
	/*初始化W5500*/
    w5500_init(); 	
	debug_printf("\r\n\r\n 网关启动 \r\n\r\n"); /*提示信息*/
	ethernet_task_state_channge(1);
	clear_connect_num();
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:  
*********************************************************************************************************
**/
void ethernet_run_reset(void)
{			
	/*以太网任务初始化复位重启*/
	ethernet_task_state_channge( 0 );	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:  
*********************************************************************************************************
**/
u8 iap_time_handle(u8 cmd);
void ethernet_run(void)
{
	static u16 connect_count = 0;	
	 /*DHCP执行*/
	switch(DHCP_run())          
	{
		case DHCP_IP_ASSIGN:     /*路由器分配ip*/
		case DHCP_IP_CHANGED:    /*路由器改变了分配ip*/
		{
			connect_count = 0;
			my_ip_assign();   	/*调用IP参数获取函数*/
		}
		break;           
		/*路由器分配ip，正式租用了，可以联网通信*/
		case DHCP_IP_LEASED:  
		{
			connect_count = 0;	
			/*使能云端上报*/
			if(user_system_data.set.cloud_selection == 1)			
				scoket_tcp_handle(user_scoket_prot_handle[0].port , user_scoket_prot_handle[0].handle); /*处理云端服务器SCOKET 数据*/
			/*端口是否关闭*/
			if(user_scoket_prot_handle[1].close_flag == 0)
				scoket_tcp_handle(user_scoket_prot_handle[1].port , user_scoket_prot_handle[1].handle); /*处理配置服务器SCOKET 数据*/
			/*端口是否关闭*/
			if(user_scoket_prot_handle[2].close_flag == 0)
				scoket_dns_handle(user_scoket_prot_handle[2].port , user_scoket_prot_handle[2].handle);/*处理DNS域名解析SCOKET 数据*/
			/*端口是否关闭*/
			if((user_scoket_prot_handle[3].close_flag == 0) && (user_system_data.set.cloud_selection == 1))
				scoket_tcp_handle(user_scoket_prot_handle[3].port , user_scoket_prot_handle[3].handle);/*处理均衡负载IP获取SCOKET 数据*/			
		}
		break;
		/*获取IP失败 */
		case DHCP_FAILED:   
		{	
			connect_count = 0;			
			my_dhcp_retry++;                   //失败次数+1
			if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
			{  
				
				debug_printf("DHCP失败，准备重启\r\n");
				ethernet_run_reset(); /* W5500初始化复位重启 */		    
				debug_printf("网络故障IP失败\r\n");
			}
		}
		break;		
	}
	connect_count++;
	if(connect_count > 3000)
	{
		connect_count = 0;
		ethernet_run_reset();
		debug_printf("*网关DHCP运行异常，复位W5500!\r\n"); /*提示信息*/		
	}
	/*延时处理*/
	cloud_data_monitor_reset();	
	/*延时处理*/
	vTaskDelay(1);
	iap_time_handle(0);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:1分钟没有收到云端数据 复位
*********************************************************************************************************
**/
void cloud_data_monitor_reset(void)
{
	ethernet_data_monitor_num++;
	/*1分钟没有收到云端数据 复位*/
	if((ethernet_data_monitor_num > 60000)&&(user_system_data.set.cloud_selection == 1))
	{
		/*以太网任务重启*/
		ethernet_task_state_channge( 0 );
		/*清零状态*/
		ethernet_data_monitor_num = 0 ;
		memset(scoket_connect_flag, 0 ,sizeof(scoket_connect_flag));
		debug_printf("网络任务重启\r\n");
		debug_printf("网络故障云端无数据\r\n");
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空计数器
*********************************************************************************************************
**/
void cloud_data_monitor_clear(void)
{
	ethernet_data_monitor_num = 0; 
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空计数器
*********************************************************************************************************
**/
u8  get_cloud_connect_state(void)
{
	if(scoket_connect_flag[SOCK_CLOUD] == 1)
		return 1;
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:关闭均衡负载
*********************************************************************************************************
**/
u8 scoket_balanced_flag = 0;
void close_balanced(u8 cmd)
{
	user_scoket_prot_handle[3].close_flag = 1;
	scoket_balanced_flag = cmd; 
	close(SOCK_BALANCED);
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:清空计数器
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
#define CONNECT_NUM 8
static u8 connect_num[CONNECT_NUM] ;
static u8 lcoal_ip_num=0;
void clear_connect_num(void)
{
	lcoal_ip_num = 0;
	for(u8 i = 0 ; i < CONNECT_NUM ; i++)
	{
		connect_num[i] = 0;
	}
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: SCOKET 连接处理   
　　TCP编程的客户端一般步骤是： 
　　1、创建一个socket，用函数socket()； 
　　2、设置要连接的对方的IP地址和端口，连接服务器，用函数connect()；
　　3、收发数据，用函数send()和recv()；
　　4、关闭网络连接；
*********************************************************************************************************
**/
#define SOCKET_DELAY (1000)

u8 scoket_tcp_handle(u8 user_port ,callback_u8_u8 scoket_fuc)
{
	static u16 scoket_delay_buff[8]={0,0,0,0,0,0,0,0}; /* 8个socket 延时*/
	static u16 scoket_restart_buff[8]={0,0,0,0,0,0,0,0}; /* 8个socket 重启计数器*/	

	int ret;
	static u16 dns_delay=0;
	if(user_port > 7) /*端口必须小于等于7*/
		return 0;
	if(scoket_delay_buff[user_port] == 0)
	{
		ret=getSn_SR(user_port);       /*获取TCP连接状态*/    
		switch(ret)     
		{
			/*处在关闭阶段，创建SOCKET*/					
			case SOCK_CLOSED:          
			{
				 debug_printf("SOCK%d准备打开本地端口\r\n",user_port);                
				 ret = socket(user_port, Sn_MR_TCP, 5188, Sn_MR_ND); /*创建一个SCOKET TCP类型*/
				/*返回值不等于当前使用端口，表示打开错误*/
				 if(ret != user_port)                             
				 {
					debug_printf("SOCK%d端口错误，准备重启\r\n",user_port);         

					scoket_delay_buff[user_port]  = SOCKET_DELAY;
					scoket_restart_buff[user_port]++;
					 /* 10次扫描后 W5500初始化重启 */
					if(scoket_restart_buff[user_port] > 3)
					{
						ethernet_run_reset();
						debug_printf("网络故障端口错误\r\n");
					}						
				 }
				 else
					 scoket_restart_buff[user_port] = 0;
				 scoket_connect_flag[user_port] = 0 ;
				 debug_printf("SOCK%d打开本地端口成功\r\n",user_port);                
			}
			break;
			 /*SOCKET初始化成功，准备侦听*/ 
			case SOCK_INIT:         
			{					
				if(user_port == SOCK_CLOUD )
				{
					 /*等待 DNS解析域名*/ 
					if(scoket_balanced_flag != 0) 			
					{						
						/*连接均衡负载*/
						if(scoket_balanced_flag == 1)
						{
							debug_printf("SOCK%d连接域名IP\r\n", user_port); 					
							ret = connect(user_port, user_scoket_balanced.dest_ip , user_scoket_balanced.dest_prot); 
						}
						/*最终IP*/
						else		
						{
							debug_printf("SOCK%d连接数字IP\r\n", user_port); 
							ret = connect(user_port, user_system_data.set.final_server_ip, user_system_data.set.final_serve_port[0]*256+user_system_data.set.final_serve_port[1]); 
						}
						connect_num[user_port]++;
						if(ret == SOCKERR_NOPEN)/*判断服务器未开启*/
						{
							debug_printf("SOCK%d服务器未开启，第%d次连接失败，10s后准备重新连接,\r\n", user_port , connect_num[user_port]);  
							scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
							/*多次连接失败，处理方法  云端10次连接失败重启  本地端5次连接失败更换IP 28 68 88 */					
							if(connect_num[user_port] > 10)
							{
								debug_printf("SOCK%d连接云端服务器失败，设备即将重启\r\n",user_port); 
								ethernet_run_reset();
								debug_printf("网络故障云端服务器故障\r\n");
							}								
						}
					}										
				}
				else if(user_port == SOCK_LOCAL )
				{
					debug_printf("SOCK%d准备连接服务器\r\n",user_port); 
					ret = connect(user_port, user_scoket_point_buff[user_port]->dest_ip, user_scoket_point_buff[user_port]->dest_prot); 

					connect_num[user_port]++;
					if(ret == SOCKERR_NOPEN)/*判断服务器未开启*/
					{
						debug_printf("SOCK%d服务器未开启，第%d次连接失败，10s后准备重新连接,\r\n", user_port , connect_num[user_port]);  
						scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
					}						
				}
				if(user_port == SOCK_BALANCED )
				{
					 /*等待 DNS解析域名*/ 
					if(dns_finish_flag == 0) 
					{
						if((dns_delay++%500) == 0)
						{
							debug_printf("第%d次等待DNS解析域名\r\n",dns_delay/300); 
						}
						if(dns_delay >= 3000)
						{
							debug_printf("DNS解析域名失败\r\n",dns_delay/300); 
							dns_finish_flag = 2;
						}
						
					}
					else			
					{							
						/*连接域名解析IP*/
						if(dns_finish_flag == 1)
						{
							debug_printf("SOCK%d连接域名IP\r\n", user_port); 					
							ret = connect(user_port, DNS_GET_IP , user_system_data.set.load_balancing_port[0]*256 + user_system_data.set.load_balancing_port[1]); 
							connect_num[user_port]++;
							if(ret == SOCKERR_NOPEN)/*判断服务器未开启*/
							{
								debug_printf("SOCK%d服务器未开启，第%d次连接失败，10s后准备重新连接,\r\n", user_port , connect_num[user_port]);  
								scoket_delay_buff[user_port]  = SOCKET_DELAY*3;
								/*多次连接失败，处理方法  0次连接失败重启 设置标志位 关闭socket  */					
								if(connect_num[user_port] > 10)
								{
									close_balanced(2);									
								}								
							}
						}
					}										
				}						
			}
			break;			
			/*连接建立成功  接收 发送数据*/ 
			case SOCK_ESTABLISHED:   
			{
				/*判断一下链接成功与否*/		
				if((scoket_connect_flag[user_port] == 0) && (getSn_IR(user_port) == Sn_IR_CON))   
				{
					scoket_connect_flag[user_port] = 1;
					debug_printf("SOCK%d连接已建立\r\n",user_port);      /*串口提示信息*/
					connect_num[user_port] = 0;
				}
				 /*接收 发送数据回调函数*/	
				(*scoket_fuc)(user_port);		
			}
			break; 
			
			/*等待关闭连接*/							 
			case SOCK_CLOSE_WAIT:      
			{
				debug_printf("SOCK%d等待关闭连接\r\n",user_port);   
				if(( ret = disconnect(user_port) ) != SOCK_OK)  /*关闭连接，并判断关闭成功与否*/
				{
					debug_printf("SOCK%d连接关闭失败，准备重启\r\n",user_port);       /*提示信息*/

					scoket_delay_buff[user_port]  = SOCKET_DELAY;
					scoket_restart_buff[user_port]++;
					if(scoket_restart_buff[user_port] > 3)
					{
						ethernet_run_reset();	                     /*重启*/
						debug_printf("网络故障连接失败\r\n");
					}
				 }
				 else
					 scoket_restart_buff[user_port] = 0;
				scoket_connect_flag[user_port] = 0; 
				debug_printf("SOCK%d连接关闭成功\r\n",user_port);         /*提示关闭成功*/
			}
			break;
		
			default: 						
			break;      
		}
	}
	else
	{
		if(scoket_delay_buff[user_port]  > 0)
			scoket_delay_buff[user_port]--;	
		vTaskDelay(1);
	}
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: SCOKET 连接处理   UDP  
*********************************************************************************************************
**/
u8 scoket_dns_handle(u8 user_port ,callback_u8_u8 scoket_fuc)
{	
	static u16 scoket_delay_buff[8]={0,0,0,0,0,0,0,0}; /* 8个socket 延时*/
	static u16 dns_wait_time = 0;
	int  ret;                       //用于保存函数返回值
	u16 len, port;
	struct dhdr dhp;
	static u8 udp_dns_buff[200];
	
	/*端口必须小于等于7*/
	if(user_port > 7) 
		return 0;
	if((scoket_delay_buff[user_port] == 0) && (dns_finish_flag == 0))
	{
		/*获取连接状态*/ 
		ret=getSn_SR(user_port);          
		switch(ret)     
		{
			/*SOCK初始化成功，可以接收数据*/   
			case SOCK_UDP:           
				
				if ((len = getSn_RX_RSR(user_port)) > 0)
				{
					if (len > MAX_DNS_BUF_SIZE) 
						len = MAX_DNS_BUF_SIZE;
					len = recvfrom(user_port, udp_dns_buff, len, dns_server_ip, &port);
					
					debug_printf("收到DNS数据%d\r\n",len);      /*串口提示信息*/
					if(parseMSG(&dhp, udp_dns_buff))
					{
						debug_printf("完成IP解析：%d.%d.%d.%d\r\n",DNS_GET_IP[0],DNS_GET_IP[1],DNS_GET_IP[2],DNS_GET_IP[3]);
						close(user_port);
						user_scoket_prot_handle[2].close_flag = 1;
						debug_printf("关闭SOCK\r\n");
						dns_finish_flag = 1;
					}
					else
					{
						debug_printf("域名解析错误\r\n");
					}
				}
				else
				{
					#define DNS_TIME_NUM 300
					if(( dns_wait_time++%DNS_TIME_NUM ) == 0)
					{
						/*向DNS服务发送域名*/
						len = dns_makequery(0, domain_name, udp_dns_buff, MAX_DNS_BUF_SIZE);
						debug_printf("第%d次向DNS发送域名%s\r\n", dns_wait_time/DNS_TIME_NUM, domain_name);    
						sendto(user_port, udp_dns_buff, len, dns_server_ip, IPPORT_DOMAIN);									
					}
					if(dns_wait_time>1000)
					{
						dns_finish_flag = 2;
						close(user_port);
						debug_printf("DNS服务器无应答，关闭端口，请检查域名！%s\r\n");  							
					}				
				}					
			break;
				
			/*SOCK处在关闭状态，可以打开*/ 
			case SOCK_CLOSED:        			
				debug_printf("SOCK%d准备打开端口\r\n",user_port);          
				/*打开socket端口 UDP 模式*/ 
				ret = socket(user_port,Sn_MR_UDP,5188,0);
				/*表示打开错误*/ 
				if(ret != user_port)                             
				{
					debug_printf("SOCK%d端口错误，准备重启\r\n",user_port);         
					scoket_delay_buff[user_port] = SOCKET_DELAY;                             
					ethernet_run_reset();	 
					debug_printf("网络故障端口关闭\r\n");                    
				}
				debug_printf("SOCK%d打开本地端口成功\r\n",user_port); 
				
				/*向DNS服务发送域名*/
				memcpy(domain_name, user_system_data.set.domain_name, user_system_data.set.domain_name_length);
				len = dns_makequery(0, domain_name, udp_dns_buff, MAX_DNS_BUF_SIZE);
				debug_printf("向DNS发送域名%s\r\n",domain_name);    
				sendto(user_port, udp_dns_buff, len, dns_server_ip, IPPORT_DOMAIN);				
				
			break;                                         

			default:               
			break;         
		}
	}
	else
	{
		if(scoket_delay_buff[user_port]  > 0)
			scoket_delay_buff[user_port]--;	
		vTaskDelay(1);
	}
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 获取到IP时的回调函数 
*********************************************************************************************************
**/
void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);     //把获取到的ip参数，记录到机构体中
   getGWfromDHCP(gWIZNETINFO.gw);     //把获取到的网关参数，记录到机构体中
   getSNfromDHCP(gWIZNETINFO.sn);     //把获取到的子网掩码参数，记录到机构体中
   getDNSfromDHCP(gWIZNETINFO.dns);   //把获取到的DNS服务器参数，记录到机构体中
   gWIZNETINFO.dhcp = NETINFO_DHCP;   //标记使用的是DHCP方式
   network_init();                    //初始化网络  
   debug_printf("DHCP租期 : %d 秒\r\n", getDHCPLeasetime());
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 获取IP的失败函数 
*********************************************************************************************************
**/
void my_ip_conflict(void)
{
	debug_printf("获取IP失败，准备重启\r\n");   //提示获取IP失败
	w5500_user_reset();                      //重启
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 初始化网络函数  
*********************************************************************************************************
**/
void network_init(void)
{
	char tmpstr[6] = {0};
	wiz_NetInfo netinfo;
	
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);//设置网络参数
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);	//读取网络参数
	ctlwizchip(CW_GET_ID,(void*)tmpstr);	        //读取芯片ID

	//打印网络参数
	if(netinfo.dhcp == NETINFO_DHCP) 
		debug_printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else 
		debug_printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);	
    debug_printf("===========================\r\n");
	debug_printf("MAC地址: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);			
	debug_printf("IP地址: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	debug_printf("网关地址: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	debug_printf("子网掩码: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	debug_printf("DNS服务器: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	debug_printf("===========================\r\n");
	
	user_scoket_set.dest_ip[0] = netinfo.ip[0];
	user_scoket_set.dest_ip[1] = netinfo.ip[1];
	user_scoket_set.dest_ip[2] = netinfo.ip[2];

}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 初始化W5500  
*********************************************************************************************************
**/
void w5500_init(void)
{
	//W5500收发内存分区，收发缓冲区各自总的空间是16K，（0-7）每个端口的收发缓冲区我们分配
    char memsize[2][8] = {{2,2,2,4,2,2,1,1},{2,2,2,4,2,2,1,1}}; 
	char tmp;
	u16 clk = 0;
	SPI_Configuration();                                    //初始化SPI接口
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//注册临界区函数
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  //注册SPI片选信号函数
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//注册读写函数
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
	{   //如果if成立，表示收发内存分区失败
		 debug_printf("初始化收发分区失败,准备重启\r\n");      //提示信息
  		 w5500_user_reset();                                //重启
	}
	/*检查连接状态*/
    do
	{                                                    
		if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)/*检查连接状态*/
		{ 		 
			debug_printf("未知错误，准备重启\r\n");            
			w5500_user_reset();                             //重启
		}
		if(tmp == PHY_LINK_OFF)
		{
			if((clk%20) == 0)
			{
				debug_printf("网线未连接\r\n");//如果检测到，网线没连接，提示连接网线
				vTaskDelay(100);              //延时
			}
			/*60秒跳出 复位重启*/
			if(clk > 600)
				break;
		}
		vTaskDelay(100);
		clk++;
	}while(tmp == PHY_LINK_OFF);                            

	/*设置MAC地址*/
	setSHAR(gWIZNETINFO.mac);                                   //设置MAC地址
	DHCP_init(SOCK_DHCP, dhcp_receive_buff);                     //初始化DHCP
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);//注册DHCP回调函数 
    my_dhcp_retry = 0;	                                        //DHCP重试次数=0
}


///***********************************************END*****************************************************/

