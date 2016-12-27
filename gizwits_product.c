/**
************************************************************
* @file         gizwits_product.c
* @brief        Gizwits 控制协议处理,及平台相关的硬件初始化
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
#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "gizwits_protocol.h"
#include "driver/hal_key.h"

taskInfo_t localtaskInfo;
/** 用户区当前设备状态结构体*/
//extern dataPoint_t currentDataPoint;
dataPoint_t currentDataPoint;
//devStatus_t localDevStatus;
/**@name Gizwits 用户API接口
* @{
*/

/**
* @brief 事件处理接口

* 说明：

* 1.用户可以对WiFi模组状态的变化进行自定义的处理

* 2.用户可以在该函数内添加数据点事件处理逻辑，如调用相关硬件外设的操作接口

* @param[in] info : 事件队列
* @param[in] data : 协议数据
* @param[in] len : 协议数据长度
* @return NULL
* @ref gizwits_protocol.h
*/
int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len)
{
  uint8_t i = 0;
  dataPoint_t *dataPointPtr = (dataPoint_t *)data;
  moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)data;

  if((NULL == info) || (NULL == data))
  {
    return -1;
  }

  for(i=0; i<info->num; i++)
  {
    switch(info->event[i])
    {
      case EVENT_ONOFF:
//        currentDataPoint.valueOnOff = dataPointPtr->valueOnOff;
        os_printf("Evt: EVENT_ONOFF %d \n", currentDataPoint.valueOnOff);
//        if(0x01 == currentDataPoint.valueOnOff)
//        {
//        	GPIO_OUTPUT_SET(BIT4, LOW_LEVEL);//user handle
//        }
//        else
//        {
//        	GPIO_OUTPUT_SET(BIT4, HIGH_LEVEL);//user handle
//        }
        do_socketOnoff(gizwitsProtocol.gizCurrentDataPoint.valueOnOff,HIGH_LEVEL);
        do_socketOnoff((~gizwitsProtocol.gizCurrentDataPoint.valueOnOff)&0x1f,LOW_LEVEL);
        system_os_post(USER_TASK_PRIO_0, SIG_SAVE_DATA, 0);
        break;

      case EVENT_TIMER:
        os_printf("Evt: EVENT_TIMER\n:%X\n",dataPointPtr->valueTimer[0]);
        //os_memcpy((uint8_t *)&currentDataPoint.valueTimer,(uint8_t *)&dataPointPtr->valueTimer,sizeof(currentDataPoint.valueTimer));
		setTimehandle(dataPointPtr);
        //user handle
		  system_os_post(USER_TASK_PRIO_0, SIG_SAVE_DATA, 0);
        break;

      case WIFI_SOFTAP:
        break;
      case WIFI_AIRLINK:
        break;
      case WIFI_STATION:
        break;
      case WIFI_CON_ROUTER:
        break;
      case WIFI_DISCON_ROUTER:
        break;
      case WIFI_CON_M2M:
    	  	os_printf("WIFI_CON_M2Mtest\n");
			CountXinqi();
			ConServer = 1;
        break;
      case WIFI_DISCON_M2M:
  	  	os_printf("WIFI_CON_M2Mtest 0\n");
			ConServer = 0;
        break;
      case WIFI_RSSI:
        os_printf("RSSI %d\n", wifiData->rssi);
        break;
      case TRANSPARENT_DATA:
        os_printf("TRANSPARENT_DATA \n");
        //user handle , Fetch data from [data] , size is [len]
        break;
      default:
        break;
    }
  }
  system_os_post(USER_TASK_PRIO_0, SIG_UPGRADE_DATA, 0);
  return 0;
}
/**@} */

void setTimehandle(dataPoint_t* setTimerBuf) {
	uint8 var = 0 ;
	control_task_t* setTimeDataIn = (control_task_t*) (((uint8_t *)setTimerBuf)+1);
	uint8_t setTimehandlevar;
	uint8_t* setTimehandlepoint;
	uint8_t task_status=setTimeDataIn->taskBox[0].Task_status;
	if((task_status==0x01)||(task_status==0x09)||(task_status==0x021)||(task_status==0x29))
		task_status=1;
	else if((task_status==0x02)||(task_status==0x12)||(task_status==0x022)||(task_status==0x32))
		task_status=3;
	else if(task_status==0x05)
		task_status=2;
	else if(task_status==0x06)
		task_status=4;
	else task_status=0;
	switch (task_status) {
	case 1:
		os_printf("set Timer Task,TaskBuf is \n");
		for (var = 0; var < sizeof(taskBox_t); ++var)
		{
			os_printf("0x%X  ", (&setTimeDataIn->taskBox[0].OnOff+var));
		}
		os_printf("\n");
		if(localtaskInfo.timertaskBox[setTimeDataIn->taskBox[0].Time_Task].Task_status==00)
			localtaskInfo.tmCount++;
		if((setTimeDataIn->taskBox[0].Time_Minute==currentDataPoint.valueSystime)&&(setTimeDataIn->taskBox[0].Time_Second!=0)&&(setTimeDataIn->taskBox[0].Task_status|0x08))
			secondCDTaskNum++;
		os_memcpy(&localtaskInfo.timertaskBox[setTimeDataIn->taskBox[0].Time_Task].OnOff,
				&setTimeDataIn->taskBox[0].OnOff,sizeof(uint8_t)*7);
		setTimehandlepoint= (uint8_t*)&setTimeDataIn->taskBox[0].OnOff;
		for(setTimehandlevar=0;setTimehandlevar<7;setTimehandlevar++)
		{
			os_printf("setTimehandlevar:%d = 0x%x\n",setTimehandlevar,*(setTimehandlepoint));
			setTimehandlepoint++;
		}

		break;
	case 2:
		if(localtaskInfo.timertaskBox[setTimeDataIn->taskBox[0].Time_Task].Task_status!=0x00)
		{
			os_memset((uint8_t*)&localtaskInfo.timertaskBox[setTimeDataIn->taskBox[0].Time_Task],0x00,sizeof(uint8_t)*7);
			os_printf("Delete Timer Task,TaskBuf is %X\n",setTimeDataIn->taskBox[0].Time_Task);
			localtaskInfo.tmCount--;
		}
		break;
	case 3:
		os_printf("set CountDown Task,TaskBuf is \n");
		for (var = 0; var < sizeof(taskBox_t); ++var) {
			os_printf("0x%X  ", *(&setTimeDataIn->taskBox[0].OnOff+var));
		}
		if(localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task].Task_status==0)
			localtaskInfo.cdCount++;

		os_memcpy(&localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task].OnOff,
				&setTimeDataIn->taskBox[0].OnOff,sizeof(taskBox_t));
		os_memcpy(&localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task].Minute_Left
				,&localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task].Time_Minute,3);
		if((setTimeDataIn->taskBox[0].Time_Minute==0)&&(setTimeDataIn->taskBox[0].Time_Second!=0)&&(setTimeDataIn->taskBox[0].Task_status|0x10))
			secondCDTaskNum++;
		if(setTimeDataIn->taskBox[0].Time_Minute!=0)
			localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task].Second_Left += (60-Systime.second);
		break;
	case 4:
		if(localtaskInfo.timertaskBox[setTimeDataIn->taskBox[0].Time_Task].Time_Task!=0xff)
		{
			os_memset((uint8_t*)&localtaskInfo.cdtaskBox[setTimeDataIn->taskBox[0].Time_Task],0x00,sizeof(uint8_t)*10);
			os_printf("Delete CountDown Task,TaskBuf is %X \n",setTimeDataIn->taskBox[0].Time_Task);
			localtaskInfo.cdCount--;
		}
		break;

	default:
		os_printf("setTimeer Data Wrong!\n");
		break;

	}

}
