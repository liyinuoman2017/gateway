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
#ifndef __FLASH_BSP_H__
#define __FLASH_BSP_H__

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

/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/
void flash_read(u8 *buff, u16 length);
void flash_write_data(u32 addr, u8 *buff, u16 length);
void flash_read_data(u32 addr, u8 *buff, u16 length);
#endif
/***********************************************END*****************************************************/