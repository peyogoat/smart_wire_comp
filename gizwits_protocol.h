/**
************************************************************
* @file         gizwits_protocol.h
* @brief        对应gizwits_protocol.c的头文件 (包含产品相关定义)
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
#ifndef _GIZWITS_PROTOCOL_H
#define _GIZWITS_PROTOCOL_H
#include <stdint.h>
#include "osapi.h"
#include "user_interface.h"
#include "user_webserver.h"


/**
* @name 协议版本号
* @{
*/
#define PROTOCOL_VERSION                        "00000004"
/**@} */

/**@name P0协议版本号
* @{
*/
#define P0_VERSION                              "00000002"
/**@} */

/**@name Product Key (产品标识码)
* @{
*/
#define PRODUCT_KEY                             "4bc40679eb664b53980d76406a8abd62"
/**@} */

#define BUFFER_LEN_MAX                          900
#define REPORT_TIME_MAX                         6000  // 6s


/**@name 定时器相关状态
* @{
*/
#define SOC_TIME_OUT                            10
#define MIN_INTERVAL_TIME                       2000                        ///< The minimum interval for sending
#define MAX_SOC_TIMOUT                          1                           ///< 1ms
#define TIM_REP_TIMOUT                          600000                      ///< 600s Regularly report
/**@} */

/**@name task相关定义
* @{
*/
#define SIG_ISSUED_DATA                         0x01                        ///< 协议处理任务
#define SIG_PASSTHROUGH                         0x02                        ///< 协议透传任务
#define SIG_IMM_REPORT                          0x09
#define SIG_DO_TASK								0xff
///< 协议立即上报任务
/**@} */

/** WiFi模组配置参数*/
typedef enum
{
  WIFI_RESET_MODE = 0x00,                     ///< WIFI模组复位
  WIFI_SOFTAP_MODE,                           ///< WIFI模组softAP模式
  WIFI_AIRLINK_MODE,                          ///< WIFI模组AirLink模式
}WIFI_MODE_TYPE_T;

/**@name 数据点相关定义
* @{
*/
#define ONOFF_BYTEOFFSET                    0
#define ONOFF_BITOFFSET                     0
#define ONOFF_LEN                           1


#define SYSTIME_RATIO                         1
#define SYSTIME_ADDITION                      0
#define SYSTIME_MIN                           0
#define SYSTIME_MAX                           1450

#define CONSUMPTION_RATIO                         1
#define CONSUMPTION_ADDITION                      1
#define CONSUMPTION_MIN                           0
#define CONSUMPTION_MAX                           65534

/**@} */

/** 可写型数据点 布尔和枚举变量 所占字节大小*/

#define COUNT_W_BIT 1






/** 事件结构体*/
typedef enum
{
  WIFI_SOFTAP = 0x00,                         ///< WiFi SOFTAP 配置事件
  WIFI_AIRLINK,                               ///< WiFi模块 AIRLINK 配置事件
  WIFI_STATION,                               ///< WiFi模块 STATION 配置事件
  WIFI_OPEN_BINDING,                          ///< WiFi模块开启绑定事件
  WIFI_CLOSE_BINDING,                         ///< WiFi模块关闭绑定事件
  WIFI_CON_ROUTER,                            ///< WiFi模块已连接路由事件
  WIFI_DISCON_ROUTER,                         ///< WiFi模块已断开连接路由事件
  WIFI_CON_M2M,                               ///< WiFi模块已连服务器M2M事件
  WIFI_DISCON_M2M,                            ///< WiFi模块已断开连服务器M2M事件
  WIFI_OPEN_TESTMODE,                         ///< WiFi模块开启测试模式事件
  WIFI_CLOSE_TESTMODE,                        ///< WiFi模块关闭测试模式事件
  WIFI_CON_APP,                               ///< WiFi模块连接APP事件
  WIFI_DISCON_APP,                            ///< WiFi模块断开APP事件
  WIFI_RSSI,                                  ///< WiFi模块信号事件
  TRANSPARENT_DATA,                           ///< 透传事件
  EVENT_ONOFF,
  EVENT_TIMER,
  EVENT_TYPE_MAX
} EVENT_TYPE_T;

/** 用户区设备状态结构体*/
#pragma pack(1)
typedef struct {
  uint8_t valueOnOff;
  uint8_t valueTimer[376];
  uint32_t valueSystime;
  uint32_t valueConsumption;
} dataPoint_t;

/** 对应协议“4.10 WiFi模组控制设备”中的标志位"attr_flags" */ 
typedef struct {
  uint8_t flagOnOff:1;
  uint8_t flagTimer:1;
} attrFlags_t;

/** 对应协议“4.10 WiFi模组控制设备”中的数据值"attr_vals" */
typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint8_t valueTimer[376];
} attrVals_t;

/** 对应协议“4.10 WiFi模组控制设备”中“P0协议区”的标志位"attr_flags" + 数据值"attr_vals" */
typedef struct {
    attrFlags_t attrFlags;
    attrVals_t  attrVals;
}gizwitsIssued_t;

/** 对应协议“4.9 设备MCU向WiFi模组主动上报当前状态”中的设备状态"dev_status" */
typedef struct {
  uint8_t wBitBuf[COUNT_W_BIT];
  uint8_t valueTimer[376];
  uint16_t valueSystime;
  uint16_t valueConsumption;
} devStatus_t;

/** 对应协议“4.9 设备MCU向WiFi模组主动上报当前状态”中的 设备状态位"dev_status"  */ 
typedef struct
{
    uint8_t action;
    devStatus_t devStatus;
}gizwitsReport_t;

/** P0 command 命令码*/
typedef enum
{
    ACTION_CONTROL_DEVICE       = 0x01,             ///< 协议4.10 WiFi模组控制设备 WiFi模组发送
    ACTION_READ_DEV_STATUS      = 0x02,             ///< 协议4.8 WiFi模组读取设备的当前状态 WiFi模组发送
    ACTION_READ_DEV_STATUS_ACK  = 0x03,             ///< 协议4.8 WiFi模组读取设备的当前状态 设备MCU回复
    ACTION_REPORT_DEV_STATUS    = 0X04,             ///< 协议4.9 设备MCU向WiFi模组主动上报当前状态 设备MCU发送
    ACTION_W2D_TRANSPARENT_DATA = 0x05,             ///< WiFi到设备MCU透传
    ACTION_D2W_TRANSPARENT_DATA = 0x06,             ///< 设备MCU到WiFi透传
} action_type_t;

/** WiFi模组工作状态*/
typedef union
{
    uint16_t value;
    struct
    {
        uint16_t            softap:1;               ///< 表示WiFi模组所处的SOFTAP模式状态，类型为bool
        uint16_t            station:1;              ///< 表示WiFi模组所处的STATION模式状态，类型为bool
        uint16_t            onboarding:1;           ///< 表示WiFi模组所处的配置状态，类型为bool
        uint16_t            binding:1;              ///< 表示WiFi模组所处的绑定状态，类型为bool
        uint16_t            con_route:1;            ///< 表示WiFi模组与路由器的连接状态，类型为bool
        uint16_t            con_m2m:1;              ///< 表示WiFi模组与云端m2m的状态，类型为bool
        uint16_t            reserve1:2;             ///< 数据位补齐
        uint16_t            rssi:3;                 ///< 表示路由的信号强度，类型为数值
        uint16_t            app:1;                  ///< 表示WiFi模组与APP端的连接状态，类型为bool
        uint16_t            test:1;                 ///< 表示WiFi模组所处的场测模式状态，类型为bool
        uint16_t            reserve2:3;             ///< 数据位补齐
    }types;
} wifi_status_t;

/** 事件队列结构体 */
typedef struct {
    uint8_t num;                                    ///< 队列成员个数
    uint8_t event[EVENT_TYPE_MAX];                  ///< 队列成员事件内容
}eventInfo_t;

/** wifi信号强度数值结构体 */
typedef struct {
    uint8_t rssi;                                   ///< WIFI信号强度
}moduleStatusInfo_t;

/** 协议处理相关结构体 */
typedef struct
{
    uint32_t timerMsCount;                          ///< 时间计数(Ms)
    uint8_t transparentBuff[BUFFER_LEN_MAX];        ///< 透传数据存储区
    uint32_t transparentLen;                        ///< 透传数据长度
    dataPoint_t gizCurrentDataPoint;                ///< 当前设备状态数据
    dataPoint_t gizLastDataPoint;                   ///< 上次上报的设备状态数据
    gizwitsReport_t reportData;                     ///< 协议上报实际数据
    eventInfo_t issuedProcessEvent;                 ///< 控制事件
}gizwitsProtocol_t;

#pragma pack(1)
typedef struct
{
	uint8_t OnOff;
	uint8_t Task_status;
	uint8_t Week_Repeat;
	uint8_t Time_Task;
	uint16_t Time_Minute;
	uint8_t Time_Second;
} taskBox_t;

#pragma pack(1)
typedef struct
{
	uint8_t OnOff;
	uint8_t Task_status;
	uint8_t Week_Repeat;
	uint8_t Time_Task;
	uint16_t Time_Minute;
	uint8_t Time_Second;
	uint16_t Minute_Left;
	uint8_t Second_Left;
} local_cdtaskBox_t;

#pragma pack(1)
typedef struct
{
	uint8_t tmCount;
	uint8_t cdCount;
	taskBox_t timertaskBox[25];
	local_cdtaskBox_t cdtaskBox[20];
} taskInfo_t;

/**@name Gizwits 用户API接口
* @{
*/
extern taskInfo_t localtaskInfo;
extern gizwitsProtocol_t gizwitsProtocol;
void gizwitsSetMode(uint8_t mode);
void gizwitsInit(void);
int ICACHE_FLASH_ATTR gizwitsHandle(dataPoint_t *dataPoint);
int32_t gizwitsPassthroughData(uint8_t * data, uint32_t len);
extern  uint16_t ICACHE_FLASH_ATTR gizProtocolExchangeBytes(uint16_t value);
/**@} */

#endif