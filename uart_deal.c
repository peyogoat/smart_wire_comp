/****************************************Copyright (c)*************************
**                               ��Ȩ���� (C), 2015-2020, Ϳѻ�Ƽ�
**
**                                 http://www.tuya.com
**
**--------------�ļ���Ϣ-------------------------------------------------------
**��   ��   ��: system.c
**��        ��: wifi���ݴ�����
**ʹ �� ˵ �� : �û�������ĸ��ļ�ʵ������
**
**
**--------------��ǰ�汾�޶�---------------------------------------------------
** ��  ��: v2.3.1
** �ա���: 2016��4��15��
** �衡��: 1:�Ż��������ݽ���

** ��  ��: v2.3
** �ա���: 2016��4��14��
** �衡��: 1:֧��MCU�̼���������

** ��  ��: v2.2
** �ա���: 2016��4��11��
** �衡��: 1:�޸Ĵ������ݽ��շ�ʽ

** ��  ��: v2.1
** �ա���: 2016��4��8��
** �衡��: 1:����ĳЩ��������֧�ֺ���ָ�����ѡ��

** ��  ��: v2.0
** �ա���: 2016��3��29��
** �衡��: 1:�Ż�����ṹ
2:��ʡRAM�ռ�
**
**-----------------------------------------------------------------------------
******************************************************************************/
#define SYSTEM_GLOBAL

#include "uart_deal.h"

unsigned char volatile wifi_queue_buf[REC_BUF_SIZE];  //���ڽ��ջ���
volatile unsigned char wifi_uart_rx_buf[DATA_LEN];         //���ڽ��ջ���

//
volatile unsigned char *queue_in;
volatile unsigned char *queue_out;
volatile unsigned short queue_total_data;                                         //��ǰ�����ֽ���

/*****************************************************************************
�������� : queue_data_init
�������� : ���г�ʼ��
������� : ��
���ز��� : ��
*****************************************************************************/
static void queue_data_init(void)
{
  queue_in = (unsigned char *)wifi_queue_buf;
  queue_out = (unsigned char *)wifi_queue_buf;
  queue_total_data = 0;
}
/*****************************************************************************
�������� : wifi_uart_write_data
�������� : ��wifi uartд����������
������� : in:���ͻ���ָ��
len:���ݷ��ͳ���
���ز��� : ��
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
�������� : get_check_sum
�������� : ����У���
������� : pack:����Դָ��
pack_len:����У��ͳ���
���ز��� : У���
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
�������� : data_handle
�������� : ����֡����
������� : Start:������ʼλ
���ز��� : ��
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
�������� : get_queue_total_data
�������� : ��ȡ����������
������� : ��
���ز��� : ��
*****************************************************************************/
static unsigned short get_queue_total_data(void)
{
  return(queue_total_data);
}
/*****************************************************************************
�������� : Queue_Read_Byte
�������� : ��ȡ����1�ֽ�����
������� : ��
���ز��� : ��
*****************************************************************************/
static unsigned char Queue_Read_Byte(void)
{
	unsigned char value;
	if(queue_total_data > 0)
	{
		//������
		if(queue_out >= (unsigned char *)(wifi_queue_buf + sizeof(wifi_queue_buf)))
		{
			//�����Ѿ���ĩβ
			queue_out = (unsigned char *)(wifi_queue_buf);
		}
		
		value = *queue_out ++;   
		queue_total_data --;
	}
	  
	return value;
}
/*****************************************************************************
�������� : wifi_uart_rx_handle
�������� : wifi���ݰ�����
������� : ��
���ز��� : ��
*****************************************************************************/
void wifi_uart_rx_handle(unsigned char value)
{
  if(queue_total_data < sizeof(wifi_queue_buf))
  {
    //���в���
    if(queue_in >= (unsigned char *)(wifi_queue_buf + sizeof(wifi_queue_buf)))
    {
      queue_in = (unsigned char *)(wifi_queue_buf);
    }
    
    *queue_in ++ = value;
    queue_total_data ++;
  }
  else
  {
    //���ݶ�����
  }
}
/*****************************************************************************
�������� : wifi_package_handle
�������� : wifi���ݰ�����
������� : ��
���ز��� : ��
*****************************************************************************/
void wifi_package_handle(void)
{
	unsigned char value;
	static unsigned short frame_type = HEAD_FIRST;
                                      //��������֡����
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
�������� : wifi_protocol_init_handle
�������� : wifi_protocol��ʼ��������
������� : ��
���ز��� : ��
*****************************************************************************/
void wifi_protocol_init_handle(void)
{
	queue_data_init();
}
