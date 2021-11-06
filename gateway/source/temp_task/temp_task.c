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
#include "start_task.h"
#include "indicator_task.h"
#include "contorl_task.h"
#include "rs458_task.h"
#include "ethernet_task.h"
#include "oled_task.h"
#include "temp_task.h"
#include "data_task.h"
/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define  I2C_PORT        (PortE)
/* LED Pin definition */
#define  I2C_SCLK_PIN         (Pin07)
#define  I2C_SDA_PIN          (Pin06)

#define  I2C_SDA_H          PORT_SetBits(I2C_PORT, I2C_SDA_PIN)
#define  I2C_SDA_L          PORT_ResetBits(I2C_PORT, I2C_SDA_PIN)
#define  I2C_SCLK_H         PORT_SetBits(I2C_PORT, I2C_SCLK_PIN)
#define  I2C_SCLK_L         PORT_ResetBits(I2C_PORT, I2C_SCLK_PIN)
#define  STH_SDA			PORT_GetBit(I2C_PORT, I2C_SDA_PIN)

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


/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void SDA_IN(void)
{
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;

    /* LED0 Port/Pin initialization */
    PORT_Init(I2C_PORT, I2C_SDA_PIN, &stcPortInit);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void SDA_OUT(void)
{   
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;

    /* LED0 Port/Pin initialization */
    PORT_Init(I2C_PORT, I2C_SDA_PIN, &stcPortInit);
}

 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void delayus(long t)
{	
	long i;
	while(t)
	{
		for(i=0;i<500;i++)
		{
		i=i;
		}
		t--;
	}
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void i2cstart1()
{	
	SDA_OUT();
	delayus(15);
	I2C_SDA_H;
	delayus(15);
	I2C_SCLK_H;
	delayus(15);
	I2C_SDA_L;
	delayus(15);
	I2C_SCLK_L;
	delayus(15);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void i2cstop1()
{	
	SDA_OUT();
	I2C_SDA_L;
	delayus(15);
	I2C_SCLK_H;
	delayus(15);
	I2C_SDA_H;
	delayus(15);
	//SDA_IN1();
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void mack_0()
{
	SDA_OUT();
	I2C_SDA_L;
	delayus(5);
	I2C_SCLK_H;
	delayus(5);
	I2C_SCLK_L;
	delayus(5);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void mnak_1()
{	
	SDA_OUT();
	I2C_SDA_H;
	delayus(5);
	I2C_SCLK_H;
	delayus(5);
	I2C_SCLK_L;
	delayus(5);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void cack1()
{	
	unsigned char i;
	SDA_IN();
	I2C_SDA_H;
	delayus(5);
	I2C_SCLK_H;
	delayus(5);
	for(i=0;i<50;i++)
	{   
		if(!STH_SDA) break;
		delayus(5);
	}
	I2C_SCLK_L;
	delayus(5);
	SDA_OUT();
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
static void i2cbw1(unsigned char dat)
{
	char i;
	SDA_OUT();
	for(i=0;i<8;i++)
	{	
		if(dat&0x80)
			I2C_SDA_H;
		else 
			I2C_SDA_L;
		dat=(dat<<1);
		delayus(5);
		I2C_SCLK_H;
		delayus(5);
		I2C_SCLK_L;
		delayus(5);
	}
		delayus(5);
		I2C_SCLK_L;
		delayus(5);
		I2C_SDA_H;
		delayus(5);	
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
unsigned char i2cbr1(void)
{
	char i;
	unsigned char dat;
	SDA_IN();
	for(i=0;i<8;i++)
	{
		delayus(5);
		I2C_SCLK_H;
		delayus(5);
		dat=(dat<<1);
		if(STH_SDA) dat=dat|0x01;
		else dat=dat&0xFE;
		I2C_SCLK_L;
		delayus(5);
	}
	return(dat);
}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
u16 readT()    
{
	u8 datL=0;
	u8 datH=0;
	u16 Tdata=0;

	i2cstart1();
	i2cbw1(0x80);
	cack1();
	i2cbw1(0xE3);	
	cack1();
	delayus(50);
	
	
	i2cstart1();	
	i2cbw1(0x81);
	cack1();
	delayus(2000);


	datH=i2cbr1();
	mnak_1();	
	datL=i2cbr1();
	mack_0();
	delayus(50);
	
	i2cstop1();
	delayus(50);
	
	Tdata=datH<<8|datL;
	return Tdata;

}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
u16 readH()    
{
	u8 datL=0;
	u8 datH=0;
	u16 Tdata=0;

	
	i2cstart1();
	i2cbw1(0x80);
	cack1();
	i2cbw1(0xE5);	
	cack1();
	delayus(50);
		
	i2cstart1();	
	i2cbw1(0x81);
	cack1();
	
	delayus(2000);
	datH=i2cbr1();
	mnak_1();
	datL=i2cbr1();
	mack_0();
	delayus(50);
	
	i2cstop1();
	delayus(50);
	Tdata=datH<<8|datL;
	return Tdata;

}
 /**
*********************************************************************************************************
* @名称	: 
* @描述	: 
* @参数	: 
* @返回	: 
*********************************************************************************************************
**/
		float temp_calc,humi_cala;
void temp_task(void *pvParameters)
{
	static uint16_t clk=0;
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;

    /* LED0 Port/Pin initialization */
    PORT_Init(I2C_PORT, I2C_SDA_PIN|I2C_SCLK_PIN, &stcPortInit);


	for(;;)
	{
		u16 temp;

		/*温湿度读取 一位小数点显示  314 为 31.4℃*/
		temp = readT();
		temp &= ~(0x0003);	// clear bits [1..0] (status bits)
		//temp_calc = -46.85+175.72*((float)temp/65536);
		temp_calc = -46.85+((float)temp*0.00268);

		temp = readH();
		temp &= ~(0x0003);	// clear bits [1..0] (status bits)
		humi_cala = ((float)temp * 0.00190735) - 6;	  
		vTaskDelay(10);
	}
}

/***********************************************END*****************************************************/

