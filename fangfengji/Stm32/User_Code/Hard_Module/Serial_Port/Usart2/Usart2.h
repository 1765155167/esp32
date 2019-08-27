#ifndef _USART2_H
#define _USART2_H

#include "global.h"
#include "stdio.h"


#define USART2_BUFFER_TX_SIZE                30        //USART1 DMA缓存大小
#define USART2_BUFFER_RX_SIZE               1050       //USART1 DMA缓存大小

#define TEST_Printf	1//打印测试数据（1）/禁止打印（0）

#if   1
    #define  Debug_printf(fmt,args...)  printf (fmt,##args)
#else
    #define  Debug_printf(fmt,args...)
#endif


//usart1参数结构体
typedef struct USART2_DATA
{
  unsigned char  usart2_TX_buffer[USART2_BUFFER_TX_SIZE];//usart2的发送缓存
  volatile unsigned char usart2_tx_DMA_free;//USART,串口2dma发送空闲标记
  unsigned char  usart2_RX_buffer[USART2_BUFFER_RX_SIZE];//usart2的接收缓存
  volatile unsigned char usart2_rx_DMA_free;//USART,串口2dma接收空闲标记
  volatile unsigned int  usart2_rx_DMA_count;//USART,串口2dma接收数据长度
  volatile unsigned int  usart2_tx_DMA_count;
  volatile unsigned char flag_delay;//空闲中断是否需要延时
} Usart2_Data_Struct;

extern Usart2_Data_Struct  Usart2_Data;//串口2数据结构体

void USART2_Configuration(void);//uart2串口初始化
void USART2_SendDMA(unsigned char* buf,unsigned short int length);//DMA方式发送字符串
void USART2_Rev_DMA_Flag_Clear(void);//DMA发送字符串标志清楚，允许再次接收命令
void Usart2_Interrupt(void);//串口1中断函数	
void USART2_Re_Data_Deal(void);//串口1接收数据处理
void USART2_TX_Data(unsigned char* buf,unsigned short int length);//串口2数据发送

#endif

