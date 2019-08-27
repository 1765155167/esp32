#ifndef _USART1_H
#define _USART1_H

#include "global.h"


#define USART1_BUFFER_TX_SIZE                30        //USART1 DMA缓存大小
#define USART1_BUFFER_RX_SIZE               1050       //USART1 DMA缓存大小

#define Usart1_Delay HAL_Delay

//usart1参数结构体
typedef struct USART1_DATA
{
	unsigned char usart1_TX_buffer[USART1_BUFFER_TX_SIZE];//usart1的发送缓存
	volatile unsigned char usart1_tx_DMA_free;//USART,串口1dma发送空闲标记
	unsigned char usart1_RX_buffer[USART1_BUFFER_RX_SIZE];//usart1的接收缓存
	volatile unsigned char usart1_rx_DMA_free;//USART,串口1dma接收空闲标记
	volatile unsigned int  usart1_rx_DMA_count;//USART,串口1dma接收数据长度
        volatile unsigned int  usart1_tx_DMA_count;
        volatile unsigned char flag_delay;//空闲中断是否需要延时
} Usart1_Data_Struct;

extern Usart1_Data_Struct  Usart1_Data;//串口1数据结构体

void USART1_Configuration(void);//uart1串口初始化
void USART1_SendDMA(unsigned char* buf,unsigned short int length);//DMA方式发送字符串
void USART1_Rev_DMA_Flag_Clear(void);//DMA发送字符串标志清楚，允许再次接收命令
void Usart1_Interrupt(void);//串口1中断函数	
void USART1_Re_Data_Deal(void);//串口1接收数据处理
void Scopy(unsigned char str1[],unsigned char str2[],unsigned int length);//将str2复制到str1
unsigned char judeg_str_equal(unsigned char* str1,unsigned char* str2,unsigned char length);//判断当2个字符串前n个字符是否相等
void USART1_TX_Data(unsigned char* buf,unsigned short int length);//串口1数据发送

#endif
