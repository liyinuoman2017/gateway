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
#include "user_type.h"

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
#define IAP_SYNCH 		0XF1
#define IAP_SYNCH_ACK 	0XF2

#define IAP_START 		0XF3
#define IAP_START_ACK 	0XF4

#define IAP_SEND 		0XF5
#define IAP_SEND_ACK 	0XF6

#define IAP_FINISH 		0XF7
#define IAP_FINISH_ACK 	0XF8

#define IAP_ERR_ACK 	0XFE
#define SECTOR_SIZE  (1024*8)

#define FLASH_SECTOR_SIZE           0x2000ul
#define FLASH_SIZE                  (64u * FLASH_SECTOR_SIZE)
#define RAM_SIZE                    0x2F000ul
#define FLASH_BASE            ((uint32_t)0x00000000) /*!< FLASH base address in the alias region */
#define SRAM_BASE             ((uint32_t)0x1FFF8000) /*!< SRAM base address in the alias region */
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
static u8 write_num = 0;
static u8 start_num = 0;
static u8 iap_receive_flag = 0;
static u8 iap_receive_num = 0;
static u8 iap_finish_num = 0;

void clear_iap_flag(void);
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 标志位检查
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
u8 app_area_detection(void)
{
	u8 ret = 0;
	uint32_t *u32Addr=(uint32_t*) (FLASH_IAP_MARK_ADRR );
	if((0x1234 == u32Addr[0]) && (0x5678 == u32Addr[1]) && (0x3141 == u32Addr[3]) && (0x5926 == u32Addr[4]))
	{
		if(u32Addr[2] == 0xa1a1)
			ret = 1;
		else if(u32Addr[2] == 0xa2a2)
			ret = 2;
	}
	return ret;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: tcp iap 用户标志
*********************************************************************************************************
**/
void boot_mark_set(void)
{
	uint32_t u32Addr;	
	u32 user_sequence[5] = {0x1234,0x5678,0xa1a1,0x3141,0x5926};

    /* Unlock EFM.*/
    EFM_Unlock();
    /* Enable flash.*/
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));
    /* Erase sector  */
    EFM_SectorErase(FLASH_IAP_MARK_ADRR);
	u32Addr = FLASH_IAP_MARK_ADRR;
    for(u8 i = 0u; i < 5; i++)
    {
        EFM_SingleProgram(u32Addr,user_sequence[i]);
        u32Addr += 4u;
    }	
    /* Lock EFM. */
    EFM_Lock();
	
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: tcp iap 清空用户标志
*********************************************************************************************************
**/
void boot_mark_clear(void)
{
    /* Unlock EFM.*/
    EFM_Unlock();
    /* Enable flash.*/
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while(Set != EFM_GetFlagStatus(EFM_FLAG_RDY));
    /* Erase sector  */
    EFM_SectorErase(FLASH_IAP_MARK_ADRR);	
    /* Lock EFM. */
    EFM_Lock();
	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 
*********************************************************************************************************
**/
u8 boot_mark_detection(void)
{
	u32 user_sequence[5] = {0x1234,0x5678,0xa1a1,0x3141,0x5926};	
	u8 ret = 0;
	uint32_t *u32Addr=(uint32_t*) (FLASH_IAP_MARK_ADRR );
	if((user_sequence[0] == u32Addr[0]) && (user_sequence[1] == u32Addr[1]) && (user_sequence[2] == u32Addr[2])
		&& (user_sequence[3] == u32Addr[3]) && (user_sequence[4] == u32Addr[4]))
	{
		ret = 1;
	}
	return ret;
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: tcp iap 升级应答
*********************************************************************************************************
**/
void tcp_iap_reply(uint8_t cmd)
{
	u8 send_buff[10];
	u8 length = 0;
	
	send_buff[length++] = 0X7E;
	send_buff[length++] = 0;	
	send_buff[length++] = 2;
	send_buff[length++] = cmd;
	send_buff[length++] = 0;
	send_buff[length++] = 0X7D;
	
	send(SOCK_LOCAL, send_buff, length);	
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:iap接收数据
*********************************************************************************************************
**/
static u32 iap_flash_sector = 0;
static u32 iap_flash_add = 0;

void iap_receive(uint8_t* reve_buff,int16_t reve_length ,uint8_t *package_num)
{
	int16_t iap_length;
	if(reve_length > 8)
	{
		debug_printf("写FLAHS地址%x\n",FLASH_CACHE_ADRR+iap_flash_add );/*打印信息*/			
		iap_length =  reve_length - 9;
		/*判断地址 擦除FLASH*/
		if(iap_flash_add >= iap_flash_sector*SECTOR_SIZE)
		{
			flash_erase(FLASH_CACHE_ADRR+iap_flash_add);
			iap_flash_sector++;
		}
		/*写FLASH 地址计数器增加*/
		flash_write(FLASH_CACHE_ADRR + iap_flash_add ,&reve_buff[7] , iap_length);
		iap_flash_add += iap_length;
		
		debug_printf("收到第%d包数据，收到%d字节\n",(*package_num), iap_length );/*打印信息*/	
		vTaskDelay(10);/*延时 系统切换任务*/
		write_num++;
		tcp_iap_reply(IAP_SEND_ACK);		
	}
	else
		debug_printf("接收数据小于8字节字节\n");/*打印信息*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	:iap完成 
*********************************************************************************************************
**/
void iap_finish(void)
{
	/* mark标志位  */
	boot_mark_set();	
	debug_printf("iap完成\n");/*打印信息*/
	vTaskDelay(5);
	tcp_iap_reply(IAP_FINISH_ACK);
	NVIC_SystemReset();	 /*重启让BOOT切换到厂家模式  （使得厂家和用户共用一个IAP流程）*/
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 升级计时
*********************************************************************************************************
**/

u8 iap_time_handle(u8 cmd)
{
	static u16 time_num = 0;
	if(cmd == 0)
	{
		time_num++;
	}
	else if(cmd == 1)
	{
		time_num = 0;
	}

	if(time_num > 5000)
	{
		time_num = 0;
		if(iap_receive_flag == 1)
		{
			debug_printf("iap超时\n");/*打印信息*/
			clear_iap_flag();
		}
	}
	return 0;
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 清标志位
*********************************************************************************************************
**/

void clear_iap_flag(void)
{
		/*复位LASFH计数*/
	iap_receive_flag = 0;
	iap_receive_num = 0;
	iap_flash_sector = 0;
	iap_flash_add = 0;
	start_num = 0;
	iap_finish_num = 0;	
	debug_printf("iap清标志位\n");/*打印信息*/
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: tcp iap 升级
*********************************************************************************************************
**/
void tcp_iap_handle(uint8_t* reve_buff,int16_t reve_length)
{
	/*记录分包数 结束时不计算*/
	if((iap_receive_flag == 1)&&(iap_finish_num == 0))
	{
		iap_receive_num++;
	}	
	if((reve_buff[0] == 0x7E) && (reve_buff[1] == 0x00)&& (reve_buff[2] == 0x02))/*判断包头*/
	{

		if(reve_buff[3] == IAP_START)/*启动升级*/
		{
			start_num++;
			if((start_num > 3 )&& (iap_receive_flag == 0))
			{
				debug_printf("iap启动\n");/*打印信息*/
				tcp_iap_reply(IAP_START_ACK);
				iap_time_handle(1);
				iap_receive_flag = 1;
				/*复位LASFH计数*/
				iap_receive_num = 0;
				iap_flash_sector = 0;
				iap_flash_add = 0;
				start_num = 0;
				iap_finish_num = 0;
				
			}	
		}
		else if(reve_buff[3] == IAP_SEND)/*接收数据*/
		{
			if(iap_receive_flag == 1)
			{
				if(reve_buff[4] == iap_receive_num)
				{			
					iap_receive(reve_buff, reve_length ,&iap_receive_num);	
					iap_time_handle(1);
				}
				else
				{
					tcp_iap_reply(IAP_ERR_ACK);
					clear_iap_flag();
				}
			}
		}		
		else if(reve_buff[3] == IAP_FINISH)/*升级结束*/
		{
			iap_finish_num++;
			if(iap_receive_flag == 1)
			{
				if((iap_receive_num == (reve_buff[4]+1)) &&(iap_receive_num > 10))
				{
					if(iap_finish_num > 3)
					{
						iap_finish();
					}
				}
				else
				{
					tcp_iap_reply(IAP_ERR_ACK);
					clear_iap_flag();
				}
			}
		}	
	}
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: 复制缓存固件数据到用户区 
*********************************************************************************************************
**/
void copy_data_to_app(void)
{
	u8 *app_cache=(u8*) (FLASH_IAP_MARK_ADRR );
	for(u8 i = 0 ; i < 11 ; i ++)
	{
		flash_erase(FLASH_USER_FITMWAVE_ADRR+i*0x2000);
		app_cache =(u8*) (FLASH_CACHE_ADRR +i*0x2000);
		flash_write( FLASH_USER_FITMWAVE_ADRR+i*0x2000  , app_cache , 0x2000);
		/*喂狗*/		
		WDT_RefreshCounter();
		debug_printf("擦除第%d扇区\n", i);/*打印信息*/
	}
	/*清零标记*/		
	boot_mark_clear();
}
/**
*********************************************************************************************************
* @名称	: 
* @描述	: 关闭外设
*********************************************************************************************************
**/
void boot_close_peripheral(void)
{
	//SysTick_Suspend();
	//rs485_usart_close();
}

/**
*********************************************************************************************************
* @名称	: 
* @描述	: IAP跳转
*********************************************************************************************************
**/
void iap_jump_to_app(void)
{
    uint32_t stack_top = *((__IO uint32_t *)FLASH_USER_FITMWAVE_ADRR);
	func_ptr_t jump_to_application;
	uint32_t app_jump_add;
    /* 判断栈顶地址有效性 */
    if ((stack_top > SRAM_BASE) && (stack_top <= (SRAM_BASE + RAM_SIZE)))
    {
		 /* 关闭外设*/
		boot_close_peripheral();
		 /* 配置跳转到用户程序复位中断入口 */
        app_jump_add = *(__IO uint32_t *)(FLASH_USER_FITMWAVE_ADRR + 4);
		/* 将地址指针强制转换成函数指针*/
        jump_to_application = (func_ptr_t)app_jump_add;
        /* 设置栈 */
        __set_MSP(*(__IO uint32_t *)FLASH_USER_FITMWAVE_ADRR);
		/* 跳转*/
        jump_to_application();
    }
}

/***********************************************END*****************************************************/
