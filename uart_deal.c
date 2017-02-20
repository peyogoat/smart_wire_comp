/****************************************Copyright (c)*************************
**                               版权所有 (C), 2015-2020, 涂鸦科技
**
**                                 http://www.tuya.com
**
**--------------文件信息-------------------------------------------------------
**文   件   名: system.c
**描        述: wifi数据处理函数
**使 用 说 明 : 用户无需关心该文件实现内容
**
**
**--------------当前版本修订---------------------------------------------------
** 版  本: v2.3.1
** 日　期: 2016年4月15日
** 描　述: 1:优化串口数据解析

** 版  本: v2.3
** 日　期: 2016年4月14日
** 描　述: 1:支持MCU固件在线升级

** 版  本: v2.2
** 日　期: 2016年4月11日
** 描　述: 1:修改串口数据接收方式

** 版  本: v2.1
** 日　期: 2016年4月8日
** 描　述: 1:加入某些编译器不支持函数指针兼容选项

** 版  本: v2.0
** 日　期: 2016年3月29日
** 描　述: 1:优化代码结构
2:节省RAM空间
**
**-----------------------------------------------------------------------------
******************************************************************************/
#define SYSTEM_GLOBAL

#include "uart_deal.h"

unsigned char volatile wifi_queue_buf[REC_BUF_SIZE];  //串口接收缓存
volatile unsigned char wifi_uart_rx_buf[DATA_LEN];         //串口接收缓存

//
volatile unsigned char *queue_in;
volatile unsigned char *queue_out;
volatile unsigned short queue_total_data;                                         //当前队列字节数

/*****************************************************************************
函数名称 : queue_data_init
功能描述 : 队列初始化
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void queue_data_init(void)
{
  queue_in = (unsigned char *)wifi_queue_buf;
  queue_out = (unsigned char *)wifi_queue_buf;
  queue_total_data = 0;
}
/*****************************************************************************
函数名称 : wifi_uart_write_data
功能描述 : 向wifi uart写入连续数据
输入参数 : in:发送缓存指针
len:数据发送长度
返回参数 : 无
*****************************************************************************/
/*
static void wifi_uart_write_data(unsigned char *in, unsigned short len)
{
  if((NULL == in) || (0 == len))
  {
    return;
  }
  
  while(len --)
  {
    uart_transmit_output(*in);
    in ++;
  }
}
*/
/*****************************************************************************
函数名称 : get_check_sum
功能描述 : 计算校验和
输入参数 : pack:数据源指针
pack_len:计算校验和长度
返回参数 : 校验和
*****************************************************************************/
static unsigned char get_check_sum(volatile unsigned char *pack, unsigned short pack_len)
{
  unsigned short i;
  unsigned char check_sum = 0;
  
  for(i = 0; i < pack_len; i ++)
  {
    check_sum += *pack ++;
  }
  
  return check_sum;
}

/*****************************************************************************
函数名称 : data_handle
功能描述 : 数据帧处理
输入参数 : Start:数据起始位
返回参数 : 无
*****************************************************************************/
static void data_handle(unsigned char val1,unsigned char val2,unsigned char val3)
{
  	int value;
	value=((255-val1)/1.0/255)*RELOAD_VALUE;//r
	r_value_h  =(value&0xff00)>>8;
	r_value_l  =value&0xff;
	value=((255-val2)/1.0/255)*RELOAD_VALUE;//w
	g_value_h  =(value&0xff00)>>8;
	g_value_l  =value&0xff;
	value=((255-val3)/1.0/255)*RELOAD_VALUE;//w
	b_value_h  =(value&0xff00)>>8;
	b_value_l  =value&0xff;
}
/*****************************************************************************
函数名称 : get_queue_total_data
功能描述 : 读取队列内数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static unsigned short get_queue_total_data(void)
{
  return(queue_total_data);
}
/*****************************************************************************
函数名称 : Queue_Read_Byte
功能描述 : 读取队列1字节数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static unsigned char Queue_Read_Byte(void)
{
	unsigned char value;
	if(queue_total_data > 0)
	{
		//有数据
		if(queue_out >= (unsigned char *)(wifi_queue_buf + sizeof(wifi_queue_buf)))
		{
			//数据已经到末尾
			queue_out = (unsigned char *)(wifi_queue_buf);
		}
		
		value = *queue_out ++;   
		queue_total_data --;
	}
	  
	return value;
}
/*****************************************************************************
函数名称 : wifi_uart_rx_handle
功能描述 : wifi数据包接收
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void wifi_uart_rx_handle(unsigned char value)
{
  if(queue_total_data < sizeof(wifi_queue_buf))
  {
    //队列不满
    if(queue_in >= (unsigned char *)(wifi_queue_buf + sizeof(wifi_queue_buf)))
    {
      queue_in = (unsigned char *)(wifi_queue_buf);
    }
    
    *queue_in ++ = value;
    queue_total_data ++;
  }
  else
  {
    //数据队列满
  }
}
/*****************************************************************************
函数名称 : wifi_package_handle
功能描述 : wifi数据包处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void wifi_package_handle(void)
{
	unsigned char value;
	static unsigned short frame_type = HEAD_FIRST;
                                      //串口数据帧长度
	unsigned short queue_data_length = get_queue_total_data();
	
	while(queue_data_length > 0)
	{
		value = Queue_Read_Byte();
		queue_data_length --; 
		switch(frame_type)
		{
			case HEAD_FIRST:
				if(value == 0x55){
				  	wifi_uart_rx_buf[frame_type] = value;
					frame_type = HEAD_SECOND;
				}
				break;
			case HEAD_SECOND:
				if(value == 0xaa){
					wifi_uart_rx_buf[frame_type] = value;
					frame_type = R_VAL;
				}else{
					frame_type = HEAD_FIRST;
				}
				break;
			case R_VAL:
				wifi_uart_rx_buf[frame_type] = value;
				frame_type = G_VAL;
				break;
			case G_VAL:
				wifi_uart_rx_buf[frame_type] = value;
				frame_type = B_VAL;
				break;
			case B_VAL:
				wifi_uart_rx_buf[frame_type] = value;
				frame_type = SUM_CHECK;
				break;
			case SUM_CHECK:
				wifi_uart_rx_buf[frame_type] = value;
				if(get_check_sum(wifi_uart_rx_buf,frame_type) == wifi_uart_rx_buf[frame_type]){
		  			data_handle(wifi_uart_rx_buf[R_VAL],wifi_uart_rx_buf[G_VAL],wifi_uart_rx_buf[B_VAL]);
				}
				frame_type = HEAD_FIRST;
				break;
		  	default:
				break;
		}
	}
}
/*****************************************************************************
函数名称 : wifi_protocol_init_handle
功能描述 : wifi_protocol初始化处理函数
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void wifi_protocol_init_handle(void)
{
	queue_data_init();
}
