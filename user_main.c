/**
************************************************************
* @file         user_main.c
* @brief        SOC版 入口文件
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
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#include "gizwits_product.h"
#include "gizwits_protocol.h"
#include "driver/hal_key.h"


#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif


uint8_t ConServer = 0;
uint8_t OneMinute = 0;
_tm Systime;
uint8_t var;
uint8_t CurXinqi=0;
static uint8_t SocketPin[5]={SOCKET_1,SOCKET_1,SOCKET_1,SOCKET_1,SOCKET_1};
uint8_t AllOnoff = 0x00;
bool isdosecondTTask=0;
bool isdosecondCDTask=0;
uint8_t secondTTaskNum=0;
uint8_t secondCDTaskNum=0;
/**@name Gagent模块相关系统任务参数
* @{
*/
#define TaskQueueLen    200                                                 ///< 消息队列总长度
LOCAL  os_event_t   TaskQueue[TaskQueueLen];                                ///< 消息队列
/**@} */

/**@name Gizwits模块相关系统任务参数
* @{
*/
#define userQueueLen    200                                                 ///< 消息队列总长度
LOCAL os_event_t userTaskQueue[userQueueLen];                               ///< 消息队列
/**@} */

/**@name 用户定时器相关参数
* @{
*/
#define USER_TIME_MS 	1000                                                    ///< 定时时间，单位：毫秒
#define SECOND_TIME_MS	1000
LOCAL os_timer_t userTimer;                                                 ///< 用户定时器结构体
LOCAL os_timer_t secondTimer;
/**@} */ 

/**@name 按键相关定义
* @{
*/
#define GPIO_KEY_NUM                            2                           ///< 定义按键成员总数
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO0_U      ///< ESP8266 GPIO 功能
#define KEY_0_IO_NUM                            0                           ///< ESP8266 GPIO 编号
#define KEY_0_IO_FUNC                           FUNC_GPIO0                  ///< ESP8266 GPIO 名称
#define KEY_1_IO_MUX                            PERIPHS_IO_MUX_MTMS_U       ///< ESP8266 GPIO 功能
#define KEY_1_IO_NUM                            14                          ///< ESP8266 GPIO 编号
#define KEY_1_IO_FUNC                           FUNC_GPIO14                 ///< ESP8266 GPIO 名称
LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];                              ///< 定义单个按键成员数组指针
LOCAL keys_typedef_t keys;                                                  ///< 定义总的按键模块结构体指针                                            ///< 定义总的按键模块结构体指针
/**@} */

/** 用户区当前设备状态结构体*/
//dataPoint_t currentDataPoint;

/**
* key1按键短按处理
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key1ShortPress(void)
{
    os_printf("#### key1 short press, default setup\n");
//    gizMSleep();
//    gizwitsSetMode(WIFI_RESET_MODE);
}

/**
* key1按键长按处理
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key1LongPress(void)
{
	uint8_t rusult;
	system_info_t info;
    os_printf("#### key1 long press, clean data\n");
//    rusult = spi_flash_erase_sector(CONFIG_SECTOR);
//    if(rusult == SPI_FLASH_RESULT_OK){
//    	os_printf("clean data ok\n");
//    }
//    else
//    	os_printf("clean data failed:%d",rusult);
    os_memset(&info , 0 ,sizeof(info));
    info.devStatus.wBitBuf[0] = 0x01;
	system_param_save_with_protect(CONFIG_SECTOR,  (void *)&info, sizeof(system_info_t));
}

/**
* key2按键短按处理
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key2ShortPress(void)
{
    os_printf("#### key2 short press, ON OFF \n");

    do_socketOnoff(0x3f,AllOnoff);
    AllOnoff = ~AllOnoff;
}

/**
* key2按键长按处理
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key2LongPress(void)
{
    os_printf("#### key2 long press, airlink mode\n");

    gizwitsSetMode(WIFI_AIRLINK_MODE);
}
/**
* key2按键大长按处理
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR key2LLongPress(void)
{
    os_printf("#### key2 long long press, softAP mode\n");

    gizwitsSetMode(WIFI_SOFTAP_MODE);
}

/**
* 按键初始化
* @param none
* @return none
*/
LOCAL void ICACHE_FLASH_ATTR keyInit(void)
{
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
                                NULL, key1LongPress, key1ShortPress);//NULL,
    singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
    		key2LLongPress, key2LongPress, key2ShortPress);//key2LLongPress,
    keys.singleKey = singleKey;
    os_printf("key1 name:%x ,key1 func:%d ,\n key2 name:%x ,key2 func:%d ,\n",singleKey[0]->gpio_name,singleKey[0]->gpio_func,singleKey[1]->gpio_name,singleKey[1]->gpio_func);
    keyParaInit(&keys);
}

//LOCAL void ICACHE_FLASH_ATTR keyInit(void)
//{
//    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
//                                key1LongPress, key1ShortPress);
//    singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
//                                key2LongPress, key2ShortPress);
//    keys.singleKey = singleKey;
//    keyParaInit(&keys);
//}

LOCAL void ICACHE_FLASH_ATTR switchInit(void)
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	if(currentDataPoint.valueOnOff!=0x3f)
		AllOnoff = 0xFF;
	do_socketOnoff(currentDataPoint.valueOnOff,0x20);
	do_socketOnoff((~currentDataPoint.valueOnOff)&0x3F,0x00);
}
/**
* 用户数据获取

* 此处需要用户实现除可写数据点之外所有传感器数据的采集,可自行定义采集频率和设计数据过滤算法
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR userTimerFunc(void)
{
/*
    currentDataPoint.valueSystime = ;//Add Sensor Data Collection
    currentDataPoint.valueConsumption = ;//Add Sensor Data Collection

*/    
    system_os_post(USER_TASK_PRIO_0, SIG_UPGRADE_DATA, 0);
}

/**
* @brief 用户相关系统事件回调函数

* 在该函数中用户可添加相应事件的处理
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR gizwitsUserTask(os_event_t * events)
{
    uint8_t i = 0;
    uint8 vchar = 0;

    if(NULL == events)
    {
        os_printf("!!! gizwitsUserTask Error \n");
    }

    vchar = (uint8)(events->par);

    switch(events->sig)
    {
    case SIG_UPGRADE_DATA:
        gizwitsHandle((dataPoint_t *)&currentDataPoint);
        break;
    case SIG_SAVE_DATA:
    	saveDeviceData();
			break;
    default:
        os_printf("---error sig! ---\n");
        break;
    }
}

/**
* @brief 程序入口函数

* 在该函数中完成用户相关的初始化
* @param none
* @return none
*/
void user_init(void)
{
    struct devAttrs attrs;
    uint32 system_free_size = 0;

    wifi_station_set_auto_connect(1);
    wifi_set_sleep_type(NONE_SLEEP_T);//set none sleep mode
    espconn_tcp_set_max_con(10);
    uart_init_3(9600,115200);
    UART_SetPrintPort(1);
    os_printf( "---------------SDK version:%s--------------\n", system_get_sdk_version());
    os_printf( "system_get_free_heap_size=%d\n",system_get_free_heap_size());

    struct rst_info *rtc_info = system_get_rst_info();
    os_printf( "reset reason: %x\n", rtc_info->reason);
    if (rtc_info->reason == REASON_WDT_RST ||
        rtc_info->reason == REASON_EXCEPTION_RST ||
        rtc_info->reason == REASON_SOFT_WDT_RST)
    {
        if (rtc_info->reason == REASON_EXCEPTION_RST)
        {
            os_printf("Fatal exception (%d):\n", rtc_info->exccause);
        }
        os_printf( "epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
                rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);
    }

    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
    {
        os_printf( "---UPGRADE_FW_BIN1---\n");
    }
    else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
    {
        os_printf( "---UPGRADE_FW_BIN2---\n");
    }

    //user init
    keyInit();
    read_lastdata();
    switchInit();
    //gizwits InitSIG_UPGRADE_DATA
    gizwitsInit();

    system_os_task(gagentProcessRun, USER_TASK_PRIO_1, TaskQueue, TaskQueueLen);

    attrs.mBindEnableTime = 0;
    attrs.mDevAttr[0] = 0x00;
    attrs.mDevAttr[1] = 0x00;
    attrs.mDevAttr[2] = 0x00;
    attrs.mDevAttr[3] = 0x00;
    attrs.mDevAttr[4] = 0x00;
    attrs.mDevAttr[5] = 0x00;
    attrs.mDevAttr[6] = 0x00;
    attrs.mDevAttr[7] = 0x00;
    os_memcpy(attrs.mstrDevHV, HARDWARE_VERSION, os_strlen(HARDWARE_VERSION));
    os_memcpy(attrs.mstrDevSV, SOFTWARE_VERSION, os_strlen(SOFTWARE_VERSION));
    os_memcpy(attrs.mstrP0Ver, P0_VERSION, os_strlen(P0_VERSION));
    os_memcpy(attrs.mstrProductKey, PRODUCT_KEY, os_strlen(PRODUCT_KEY));
    os_memcpy(attrs.mstrProtocolVer, PROTOCOL_VERSION, os_strlen(PROTOCOL_VERSION));
    os_memcpy(attrs.mstrSdkVerLow, SDK_VERSION, os_strlen(SDK_VERSION));
    gagentInit(attrs);

    system_os_task(gizwitsUserTask, USER_TASK_PRIO_0, userTaskQueue, userQueueLen);

    //user timer 
    os_timer_disarm(&userTimer);
    os_timer_setfn(&userTimer, (os_timer_func_t *)taskTimerTick, NULL);
    os_timer_arm(&userTimer, USER_TIME_MS, 1);

    os_printf("--- system_free_size = %d ---\n", system_get_free_heap_size());

}

/**
 * peyo
 */
void read_lastdata(void)
{
	system_info_t info;
    os_memset(&info,0,sizeof(system_info_t));
    os_memset(&currentDataPoint,0,sizeof(dataPoint_t));
    os_memset(&localtaskInfo,0,sizeof(taskInfo_t));
	system_param_load(CONFIG_SECTOR, 0, (void *)&info, sizeof(system_info_t));
	os_memcpy((uint8_t *)&currentDataPoint.valueOnOff, (uint8_t *)&info.devStatus.wBitBuf, 1);
	os_memcpy((uint8_t *)&currentDataPoint.valueSystime, (uint8_t *)&info.devStatus.valueSystime, 4);
	os_memcpy((uint8_t *)&localtaskInfo.timertaskBox, (uint8_t *)&info.devStatus.valueTimer, sizeof(uint8_t)*TIMER_SIZE);
	os_memcpy((uint8_t *)&localtaskInfo, (uint8_t *)&info.tmCount, sizeof(uint8_t)*2);
	do_Task();
}

void ICACHE_FLASH_ATTR taskTimerTick(void)
{
	uint16_t reporttask_p=0;
	static uint32 free_heap;
	system_os_post(USER_TASK_PRIO_0, SIG_UPGRADE_DATA, 0);
    Systime.ntp++;
    Systime.second++;


//    if(!(Systime.second%10))
//    os_printf("taskTimerTick! Second is:%d\n",Systime.second);
    if (Systime.second == 60)
    {
    	free_heap = system_get_free_heap_size();
    	os_printf("free_heap is :%d!!!\n",free_heap);
    	//system_print_meminfo();
        Systime.second = 0;
        Systime.minute++;
        gizwitsProtocol.gizCurrentDataPoint.valueSystime++;
        currentDataPoint.valueSystime =
        		gizwitsProtocol.gizCurrentDataPoint.valueSystime;
        if (Systime.minute == 60)
        {
            Systime.minute = 0;
            Systime.hour++;
            if (Systime.hour == 24)
            {
                Systime.hour = 0;
                if(ConServer){
                gagentGetNTP(&Systime);
                CountXinqi();
                }
                CurXinqi++;
                if(CurXinqi==7)
                	CurXinqi=1;
                Systime.day++;
            }
        }
        reporttask_p = do_Task();
   //  system_os_post(USER_TASK_PRIO_2, SIG_DO_TASK, 0);
    }
    Do_secondTask(reporttask_p);

}

void CountXinqi(void)
{
	static bool bootfirst=1;
	uint16_t Midmonth,MidYear;
	uint8_t century =20;
	if(bootfirst){
		gagentGetNTP(&Systime);
		bootfirst=0;
	}
	Midmonth = Systime.month;
	MidYear = Systime.year;
   if(Midmonth==1)
   {
           MidYear-=1;
           Midmonth=13;
   }
   if(Midmonth==2)
   {
           MidYear-=1;
           Midmonth=14;
   }
   CurXinqi=(MidYear%100+(MidYear%100/4)+(century/4)-2*century+(26*(Midmonth+1)/10)+ Systime.day-1)%7;//m_m2w_mcuTime.time_r.Time_day

    if(CurXinqi<=0)
    {
        CurXinqi+=7;//CurXinqi是几就是星期几
    }
    os_printf("CountXinqi Hour:%d , Minute:%d , CurXInqi:%d\n",Systime.hour,Systime.minute,CurXinqi);
    gizwitsProtocol.gizCurrentDataPoint.valueSystime =
    		Systime.hour*60+Systime.minute;
    currentDataPoint.valueSystime =
    		gizwitsProtocol.gizCurrentDataPoint.valueSystime;
    os_printf("CountXinqi Curtime is:%d\n",currentDataPoint.valueSystime);
}


uint16_t ICACHE_FLASH_ATTR do_Task(void)
{
	uint8_t DoSocket = 1;
	uint16_t DT_position=0;
	uint8_t DT_TaskNum[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	uint8_t taskAmount=0;
	uint8_t *p_DT_TaskNum=&DT_TaskNum[0];
	os_printf("do_task test!\n");
	os_memset(&currentDataPoint.valueTimer[0], 0, sizeof(currentDataPoint.valueTimer));
	DT_position=CountDownTask(0,p_DT_TaskNum);
	os_printf("test 2\n");
	DT_position=RealTimeTask(DT_position,p_DT_TaskNum);
	os_printf("test 2\n");
//	for(DoSocket = 1; DoSocket<6 ;DoSocket++){
//		if(DT_TaskNum[DoSocket]!=0xff){
//			if(DT_TaskNum[DoSocket]&0x80)
//				{
//				DoCDTask(p_DT_TaskNum[DoSocket]);
//				os_printf("Do CD Task last Data:0x%X",DT_TaskNum[DoSocket]);
//				}
//			else
//				{
//				DoTimerTask(p_DT_TaskNum[DoSocket]);
//				os_printf("Do Tier Task last Data:0x%X",DT_TaskNum[DoSocket]);
//				}
//		}
//	}
//	if(DT_TaskNum[0]!=0xff)
//	{
//		saveDeviceData();
//	}
	return DT_position;
}
/**
 * peyo
 */
uint16_t ICACHE_FLASH_ATTR CountDownTask(uint16_t CDT_position,uint8_t* p_CDT_TaskNum)
{
//	uint8_t CDT_DotaskNum=CDT_position;
	dataPoint_t* reportData = (dataPoint_t*) &currentDataPoint ;
	uint8_t CountDownTaskvar;
	uint8_t* CountDownTaskpoint;
	uint8_t find_socket = 0;
	uint16_t CDT_p = CDT_position;
	for(var=0;var<CDTASK_COUNT;var++)
	{
			if(localtaskInfo.cdtaskBox[var].Task_status&0x10)
			{
				CountDownTaskpoint=(uint8_t*)&localtaskInfo.cdtaskBox[var];
				os_printf("Do CountDownTask cdtaskBox[%d]is:\n",var);
				for(CountDownTaskvar=0;CountDownTaskvar<7;CountDownTaskvar++)
				{
					os_printf("0x%x\n",*CountDownTaskpoint);
					CountDownTaskpoint++;
				}
				if((localtaskInfo.cdtaskBox[var].Minute_Left)||(localtaskInfo.cdtaskBox[var].Second_Left))
				{
					//*p_CDT_TaskNum=0;
					if(localtaskInfo.cdtaskBox[var].Minute_Left)
						localtaskInfo.cdtaskBox[var].Minute_Left = gizProtocolExchangeBytes(gizProtocolExchangeBytes(localtaskInfo.cdtaskBox[var].Minute_Left)-1);
					os_printf("There is a CD task here!\nTask No is :%d \n",localtaskInfo.cdtaskBox[var].Time_Task);
					if(localtaskInfo.cdtaskBox[var].Minute_Left!=0){
						os_memcpy((uint8_t*)&reportData->valueTimer[CDT_position],&localtaskInfo.cdtaskBox[var],sizeof(local_cdtaskBox_t));
						CDT_position=+10;
					}
					os_printf("CountDownTask,copy done!\nTime_Minute is:%d\nTime_Minute_Left is :%d",gizProtocolExchangeBytes(localtaskInfo.cdtaskBox[var].Time_Minute),
							localtaskInfo.cdtaskBox[var].Minute_Left);
					if(!localtaskInfo.cdtaskBox[var].Minute_Left)
					{
						localtaskInfo.cdtaskBox[var].Second_Left++;
						secondCDTaskNum++;
						os_printf("CountDownTask DO task!\n");
//						for(find_socket=0;find_socket<5;find_socket++){
//							if(localtaskInfo.cdtaskBox[var].OnOff>>find_socket)//标识为CD task
//								p_CDT_TaskNum[find_socket+1]=var|0x80;
//						}
//						localtaskInfo.cdtaskBox[var].Task_status &= 0xEF;

					}
					else
					{

						;
					}
				}
			}
	}

	os_printf("finish checking the CDtask!\n");
	return CDT_position;
}

void ICACHE_FLASH_ATTR DoCDTask(uint8_t Num)
{
	Num&=0x7F;
	do_socketOnoff(localtaskInfo.cdtaskBox[Num].OnOff
			,localtaskInfo.cdtaskBox[Num].Task_status);
	os_printf("DoCDTask\n");
}

/**
 * peyo
 */
uint8_t ICACHE_FLASH_ATTR RealTimeTask(uint16_t RTT_position,uint8_t* p_RTT_TaskNum)
{
	uint8_t find_socket=0;
	uint8_t RTT_DotaskNum=0xff;
	dataPoint_t* reportData = (dataPoint_t*) &currentDataPoint ;
	uint16_t realtimetaskminute;
	for(var=0;var<TTASK_COUNT;var++)
	{
		if((localtaskInfo.timertaskBox[var].Task_status&0x08)&&(localtaskInfo.timertaskBox[var].Task_status!=0xFF))
		{
			if(((localtaskInfo.timertaskBox[var].Week_Repeat>>(CurXinqi-1)&&0x01))||(localtaskInfo.timertaskBox[var].Week_Repeat==0x00))
			{
				os_printf("There is Timer tasks today!,taskNois:%d\n",localtaskInfo.timertaskBox[var].Time_Task);
				os_printf("Task_status:0x%x\nTime_minute:0x%x\n",localtaskInfo.timertaskBox[var].Task_status,(uint16_t)localtaskInfo.timertaskBox[var].Time_Minute);
				os_memcpy((uint8_t*)&realtimetaskminute,(uint8_t*)&localtaskInfo.timertaskBox[var].Time_Minute,sizeof(uint8_t)*2);
				//				realtimetaskminute=Y2X(CURTIME_RATIO,CURTIME_ADDITION,realtimetaskminute);
				if(realtimetaskminute||localtaskInfo.timertaskBox[var].Time_Second)
				{
					realtimetaskminute=gizProtocolExchangeBytes(realtimetaskminute);
					os_printf("TaskTimeMinute is 0x%x \nSystimeMinute is 0x%x ;%d\n",realtimetaskminute,gizwitsProtocol.gizCurrentDataPoint.valueSystime,gizwitsProtocol.gizCurrentDataPoint.valueSystime);//,*((uint8_t*)&Systime.minute+1)
					if(realtimetaskminute == gizwitsProtocol.gizCurrentDataPoint.valueSystime)
					{
						os_printf("TimerTask Time's out No is:%d\n",localtaskInfo.timertaskBox[var].Time_Task);
						secondTTaskNum++;
//						for(find_socket=0;find_socket<5;find_socket++){
//							if(localtaskInfo.timertaskBox[var].OnOff>>find_socket)
//								p_RTT_TaskNum[find_socket+1]=var;
//						}
//
//						os_memcpy(&reportData->valueTimer[RTT_position],&localtaskInfo.timertaskBox[var],sizeof(uint8_t)*7);
//						RTT_position=+7;
//						if(localtaskInfo.timertaskBox[var].Week_Repeat==0x00)
//						{
//							localtaskInfo.timertaskBox[var].Task_status &= 0xF7;
//							*p_RTT_TaskNum = 0;
////						    system_param_save_with_protect(CONFIG_TASK,  (void *)&localtaskInfo, sizeof(localtaskInfo_t));/old
//						}

						break;
					}
				}
			}
		}
	}
	os_printf("finish checking the timertask!\n");
	return RTT_position;
}


void ICACHE_FLASH_ATTR DoTimerTask(uint8_t Num)
{
	os_printf("DoTimerTask set switch!Numis %d \n"
			,localtaskInfo.timertaskBox[Num].Time_Task);
	os_printf("Minute:%d,Second:%d,Status:0x%X\n"
			,gizProtocolExchangeBytes(localtaskInfo.timertaskBox[Num].Time_Minute)
			,localtaskInfo.timertaskBox[Num].Time_Second
			,localtaskInfo.timertaskBox[Num].Task_status);
	do_socketOnoff(localtaskInfo.timertaskBox[Num].OnOff
			,localtaskInfo.timertaskBox[Num].Task_status);
}

void ICACHE_FLASH_ATTR do_socketOnoff(uint8_t socket_num,uint8_t On_or_Off){
	uint8_t find_dosocket = 0;
	for(find_dosocket = 0; find_dosocket<5 ;find_dosocket++){
		if((socket_num>>find_dosocket)&0x01){
			if(On_or_Off&0x20){
				GPIO_OUTPUT_SET(SocketPin[find_dosocket], 0);
				currentDataPoint.valueOnOff |= (0x01<<find_dosocket);
			}
			else{
				GPIO_OUTPUT_SET(SocketPin[find_dosocket], 1);
				currentDataPoint.valueOnOff &= (~(0x01<<find_dosocket));
			}
			os_printf("do_socketOnoff!\nsocket_num:0x%X  On_or_Off:0x%X\n",
					socket_num,On_or_Off);
		}


	}
	if(currentDataPoint.valueOnOff==0x1f)
	{
		currentDataPoint.valueOnOff = 0x3f;
		AllOnoff = 0x00;
	}
	else
	{
		AllOnoff = 0xff;
	}
}

void saveDeviceData(void){
	system_info_t info;
	os_printf("saveDeviceData\n");
	os_memset(&info,0,sizeof(system_info_t));
	os_memcpy(&info.tmCount,&localtaskInfo.tmCount,2);
	os_memcpy(&info.devStatus.wBitBuf,&currentDataPoint.valueOnOff,1);
	os_memcpy(&info.devStatus.valueTimer,&localtaskInfo.timertaskBox,TIMER_SIZE);
	os_memcpy(&info.devStatus.valueSystime,&currentDataPoint.valueSystime,4);
	os_printf("saveDeviceData last\n");
	system_param_save_with_protect(CONFIG_SECTOR,  (void *)&info, sizeof(system_info_t));
}


void ICACHE_FLASH_ATTR Do_secondTask(uint16_t reportsecondTask_p)
{
    uint8_t tmp_task_count;
    uint8_t tmp_socket_count;
	uint8_t DsT_TaskNum[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	dataPoint_t* reportData = (dataPoint_t*) &currentDataPoint;
	uint16 taskTimerReport_p=reportsecondTask_p;
	if(secondTTaskNum||secondCDTaskNum)
		os_memset(&reportData->valueTimer[0],0,TIMER_SIZE);
	if(secondTTaskNum){
		os_printf("it is Do_secondTTask\n");
		os_printf("Task Minute:%d\nSystime Minute:%d\ntask status is:0x%X\ntask Second :%d \nSys Second:%d \n",
				gizProtocolExchangeBytes(localtaskInfo.timertaskBox[tmp_task_count].Time_Minute),
				currentDataPoint.valueSystime,
				localtaskInfo.timertaskBox[tmp_task_count].Task_status,
				localtaskInfo.timertaskBox[tmp_task_count].Time_Second,
				Systime.second);
		for(tmp_task_count=0;tmp_task_count<TTASK_COUNT;tmp_task_count++){
			if((localtaskInfo.timertaskBox[tmp_task_count].Time_Second==Systime.second)
					&&(gizProtocolExchangeBytes(localtaskInfo.timertaskBox[tmp_task_count].Time_Minute)==currentDataPoint.valueSystime)
					&&(localtaskInfo.timertaskBox[tmp_task_count].Task_status|0x80)){
				os_printf("Do_secondTask DO Timer task!\n task Time:%d\nSystime:%d\n",localtaskInfo.timertaskBox[tmp_task_count].Time_Minute,currentDataPoint.valueSystime);
				os_memcpy(&reportData->valueTimer[taskTimerReport_p],&localtaskInfo.timertaskBox[tmp_task_count],sizeof(uint8_t)*7);
				taskTimerReport_p=+7;
				isdosecondCDTask=1;
				secondTTaskNum--;
				for(tmp_socket_count=0;tmp_socket_count<5;tmp_socket_count++){
					if(localtaskInfo.timertaskBox[tmp_task_count].OnOff>>tmp_socket_count)//标识为Timer task
						DsT_TaskNum[tmp_socket_count+1]=tmp_task_count;
				}
				if(localtaskInfo.timertaskBox[tmp_task_count].Week_Repeat==0x00)
				{
					localtaskInfo.timertaskBox[tmp_task_count].Task_status &= 0xF7;
					DsT_TaskNum[0] = 0;
				}

			}
		}
	}

	if(secondCDTaskNum){
		for(tmp_task_count=0;tmp_task_count<CDTASK_COUNT;tmp_task_count++){
			if((localtaskInfo.cdtaskBox[tmp_task_count].Minute_Left==0)&&(localtaskInfo.cdtaskBox[tmp_task_count].Task_status&0x10)){
				os_printf("plus the seconds\n,task_status is 0x%X\ntmp_task_count is %d\nsecondCDTaskNum is %d\n"
						,localtaskInfo.cdtaskBox[tmp_task_count].Task_status,tmp_task_count,secondCDTaskNum);
				if(localtaskInfo.cdtaskBox[tmp_task_count].Second_Left)
					localtaskInfo.cdtaskBox[tmp_task_count].Second_Left--;
				os_printf("the seconds is %d now\n",
						localtaskInfo.cdtaskBox[tmp_task_count].Second_Left);
				if(!localtaskInfo.cdtaskBox[tmp_task_count].Second_Left){
					os_printf("Do_secondTask DO CD task!\nSecondLeft is:%d\n",localtaskInfo.cdtaskBox[tmp_task_count].Second_Left);
//					localtaskInfo.cdtaskBox[tmp_task_count].Minute_Left = gizProtocolExchangeBytes(gizProtocolExchangeBytes(localtaskInfo.cdtaskBox[tmp_task_count].Minute_Left)-1);
					os_printf("There is a CD task here!\nTask No is :%d \n",localtaskInfo.cdtaskBox[tmp_task_count].Time_Task);
					os_memcpy((uint8_t*)&reportData->valueTimer[taskTimerReport_p],&localtaskInfo.cdtaskBox[tmp_task_count],sizeof(local_cdtaskBox_t));
					taskTimerReport_p=+10;
					isdosecondCDTask=1;
					if(secondCDTaskNum)
						secondCDTaskNum--;
					for(tmp_socket_count=0;tmp_socket_count<5;tmp_socket_count++){
						if(localtaskInfo.cdtaskBox[tmp_task_count].OnOff>>tmp_socket_count)//标识为CD task
							DsT_TaskNum[tmp_socket_count+1]=tmp_task_count|0x80;
					}
					localtaskInfo.cdtaskBox[tmp_task_count].Task_status &= 0xEF;
					DsT_TaskNum[0]=0;
				}
			}
		}
	}

	if(isdosecondTTask||isdosecondCDTask){
		for(tmp_socket_count = 1; tmp_socket_count<6 ;tmp_socket_count++){
			if(DsT_TaskNum[tmp_socket_count]!=0xff){
				if(DsT_TaskNum[tmp_socket_count]&0x80)
					{
					DoCDTask(DsT_TaskNum[tmp_socket_count]);
					os_printf("Do CD Task last Data:0x%X",DsT_TaskNum[tmp_socket_count]);
					}
				else
					{
					DoTimerTask(DsT_TaskNum[tmp_socket_count]);
					os_printf("Do Tier Task last Data:0x%X",DsT_TaskNum[tmp_socket_count]);
					}
			}
			isdosecondCDTask=0;
			isdosecondTTask=0;
		}

	}
	if(DsT_TaskNum[0]!=0xff)
	{
		saveDeviceData();
	}


}
