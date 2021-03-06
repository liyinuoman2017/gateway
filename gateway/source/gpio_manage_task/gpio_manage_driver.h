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
#ifndef __SWITCH_DRIVER_H__
#define __SWITCH_DRIVER_H__

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
void gpio_driver_init(void);
void  switch_driver_write(u8 cmd , u8 channel ,u8 value);
u8 read_single(u8* buff);
void button_scanf(void);
#endif
/***********************************************END*****************************************************/
