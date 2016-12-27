/**
************************************************************
* @file         gizwits_product.h
* @brief        对应gizwits_product.c的头文件(包含产品软硬件版本定义)
* @author       Gizwits
* @date         2016-09-05
* @version      V03010201
* @copyright    Gizwits
* 
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#ifndef _GIZWITS_PRODUCT_H_
#define _GIZWITS_PRODUCT_H_

#include "osapi.h"
#include <stdint.h>
#include "gizwits_protocol.h"
#include "eagle_soc.h"

#define CONFIG_SECTOR 0x3F6//3f9
#define HIGH_LEVEL 0xff
#define LOW_LEVEL 0x00
#define SOCKET_1 4
#define SOCKET_2
#define SOCKET_3
#define SOCKET_4
#define USB
/**
* MCU硬件版本号
*/
#define HARDWARE_VERSION                        "03000001"
/**
* MCU软件版本号
*/
#define SOFTWARE_VERSION                        "03000201"
/**
* gagent小版本号，用于OTA升级
*/
#define SDK_VERSION                             "13"

#ifndef SOFTWARE_VERSION
    #error "no define SOFTWARE_VERSION"
#endif

#ifndef HARDWARE_VERSION
    #error "no define HARDWARE_VERSION"
#endif

#define SIG_UPGRADE_DATA 0x01
#define SIG_SAVE_DATA  0x02



//#pragma pack(1)
//typedef struct
//{
//	uint8_t OnOff;
//	uint8_t Task_status;
//	uint8_t Week_Repeat;
//	uint8_t Time_Task;
//	uint16_t Time_Minute;
//	uint8_t Time_Second;
//} taskBox_t;

//#pragma pack(1)
//typedef struct
//{
//	uint8_t OnOff;
//	uint8_t Task_status;
//	uint8_t Week_Repeat;
//	uint8_t Time_Task;
//	uint16_t Time_Minute;
//	uint8_t Time_Second;
//	uint16_t Minute_Left;
//	uint8_t Second_Left;
//} local_cdtaskBox_t;

#pragma pack(1)
typedef struct
{
	taskBox_t taskBox[45];
} control_task_t;

#pragma pack(1)
typedef struct
{
	uint8_t tmCount;
	uint8_t cdCount;
	devStatus_t devStatus;
} system_info_t;

extern bool isneedsecondTTask;
extern bool isneedsecondCDTask;
extern uint8_t secondCDTaskNum;
extern uint8_t secondTTaskNum;
#pragma pack()
/**@name Gizwits 用户API接口
* @{
*/
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len);
/**@} */
//extern devStatus_t localDevStatus;
void read_lastdata(void);
void setTimehandle(dataPoint_t* setTimerBuf);
//void ICACHE_FLASH_ATTR do_socketOnoff(uint8_t socket_num,uint8_t On_or_Off);
void ICACHE_FLASH_ATTR taskTimerTick(void);
void  CountXinqi(void);
uint16_t ICACHE_FLASH_ATTR CountDownTask(uint16_t CDT_position,uint8_t* p_CDT_TaskNum);
void ICACHE_FLASH_ATTR DoCDTask(uint8_t Num);
uint8_t ICACHE_FLASH_ATTR RealTimeTask(uint16_t RTT_position,uint8_t* p_RTT_TaskNum);
void ICACHE_FLASH_ATTR DoTimerTask(uint8_t Num);
extern void saveDeviceData(void);
void ICACHE_FLASH_ATTR do_socketOnoff(uint8_t socket_num,uint8_t On_or_Off);
void ICACHE_FLASH_ATTR Do_secondTask(uint16_t);
uint16_t ICACHE_FLASH_ATTR do_Task(void);

#endif
