/**
*********************************************************************************************************
*                                        multi_switch_control
*                                      (c) Copyright 2021-2031
*                                         All Rights Reserved
*
* @File    : 
* @By      : liwei
* @Version : V0.01
*********************************************************************************************************
**/
/*
*********************************************************************************************************
Includes 
*********************************************************************************************************
*/
#include "hc32f46x_clk.h"
#include "hc32f46x_gpio.h"
#include "gpio_manage_bsp_hc32f64.h"

/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/
#define OUTPUT_NUM 6
static en_port_t uesr_output_port[OUTPUT_NUM] =
{
	PortC,
	PortC,
	PortC,
	PortD,	
	PortB,
	PortB,
};

static en_pin_t uesr_output_pin[OUTPUT_NUM]=
{
	Pin10,
	Pin11,
	Pin12,
	Pin02,
	Pin03,
	Pin08,
};

#define INPUT_NUM 6
static en_port_t uesr_input_port[INPUT_NUM] =
{
	PortA,
	PortA,
	PortA,
	PortA,	
	PortC,
	PortB,	
};

static en_pin_t uesr_input_pin[INPUT_NUM]=
{
	Pin04,
	Pin05,
	Pin06,
	Pin07,
	Pin04,
	Pin04,	
};
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
* @√˚≥∆	: 
* @√Ë ˆ	: 
*********************************************************************************************************
**/
void gpio_bsp_init(void)
{
    stc_port_init_t stcPortInit;
	/* PB3*/
	PORT_DebugPortSetting(TDO_SWO,Disable);
    /*  ‰≥ˆIO≈‰÷√*/
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;
    /* Pin initialization */
	for( u8 i = 0 ; i < OUTPUT_NUM ; i++ )	
		PORT_Init(uesr_output_port[i], uesr_output_pin[i], &stcPortInit);	
	
    /*  ‰»ÎIO≈‰÷√*/
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    /* Pin initialization */
	for( u8 i = 0 ; i < INPUT_NUM ; i++ )	
		PORT_Init(uesr_input_port[i], uesr_input_pin[i], &stcPortInit);	
		
}
/**
*********************************************************************************************************
* @√˚≥∆	: 
* @√Ë ˆ	: 
*********************************************************************************************************
**/
void gpio_bsp_write(u8 channel , u8 value)
{
	if(channel < OUTPUT_NUM)
	{
		if(value != 0)
			PORT_SetBits(uesr_output_port[channel], uesr_output_pin[channel]);
		else
			PORT_ResetBits(uesr_output_port[channel], uesr_output_pin[channel]);	
	}	
}
/**
*********************************************************************************************************
* @√˚≥∆	: 
* @√Ë ˆ	: 
*********************************************************************************************************
**/
u8 gpio_bsp_read(u8 channel)
{
	u8 i;
	if(channel < INPUT_NUM)
		i =  PORT_GetBit(uesr_input_port[channel], uesr_input_pin[channel]);
	return i;
}

/***********************************************END*****************************************************/
