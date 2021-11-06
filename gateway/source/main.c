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
#include "FreeRTOS.h"
#include "task.h"
#include "start_task.h"
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
u8 firmware_version = 3;
/*
*********************************************************************************************************
Function 
*********************************************************************************************************
*/


/**
*********************************************************************************************************
* @����	: 
* @����	: MAIN���� ������������
*********************************************************************************************************
**/	
int32_t main(void)
{
	/* ������������ */
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������ 	
	/* ������� */			
	vTaskStartScheduler();				
    while(1);
}

/***********************************************END*****************************************************/

