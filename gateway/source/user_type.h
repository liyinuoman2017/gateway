/**
*********************************************************************************************************
*                                        multi_switch_control
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


/*
*********************************************************************************************************
Define
*********************************************************************************************************
*/


#define TASK_INIT 	0
#define TASK_RUN 	1
#define TASK_STATE0 	0
#define TASK_STATE1 	1
#define TASK_STATE2 	2
#define TASK_STATE3		3
#define TASK_STATE4 	4
/*数据流最大数量*/
#define DATA_STREAM_BUFF_MAX 50

#define TASK_STATE_NUM_1     1
#define TASK_STATE_NUM_2     2
#define TASK_STATE_NUM_3     3
#define TASK_STATE_NUM_4     4



/*
*********************************************************************************************************
Typedef
*********************************************************************************************************
*/
/*数据类型*/
typedef  int	  int32_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int	  uint32_t;
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef void (*callback_void_void)(void);

typedef u8 (*callback_u8_void)(void);
typedef u8 (*callback_u8_u8)(u8 data);

typedef char int8;

typedef volatile char vint8;

typedef unsigned char uint8;

typedef volatile unsigned char vuint8;

typedef int int16;

typedef unsigned short uint16;

typedef long int32;

typedef unsigned long uint32;

typedef uint8			u_char;		/**< 8-bit value */
typedef uint8 			SOCKET;
typedef uint16			u_short;	/**< 16-bit value */
typedef uint16			u_int;		/**< 16-bit value */
typedef uint32			u_long;		/**< 32-bit value */

/*通用数据流类型*/
typedef struct data_stream_def
{		   								
	u8 cmd;
	u8 buff[DATA_STREAM_BUFF_MAX];
}data_stream_t;

/*操作函数模板 */
typedef  u8 (*fun_template_t)(data_stream_t* stream ,u8* parameter );

/*类操作模板*/
typedef struct interface_template_def
{	
	/*读取数据操作*/	
	fun_template_t read;
	/*读取数据操作*/
	fun_template_t write;
	/*初始化操作*/
	fun_template_t initialization;
	/*中断操作  用于计数 置标志位  放在中断函数中 高实时性*/
	fun_template_t interrput;
	/*连续运行操作 */
	fun_template_t  run;
}interface_template_t;

/*操作函数模板 */
typedef  u8 (*fun_bsp_t)(data_stream_t* stream );
/*类操作模板*/
typedef struct interface_bsp_def
{	
	/*读取数据操作*/	
	fun_bsp_t read;
	/*读取数据操作*/
	fun_bsp_t write;
	/*初始化操作*/
	fun_bsp_t initialization;
	/*中断操作  用于计数 置标志位  放在中断函数中 高实时性*/
	fun_bsp_t interrput;
	/*连续运行操作 */
	fun_bsp_t  run;
}interface_bsp_t;
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

/***********************************************END*****************************************************/


