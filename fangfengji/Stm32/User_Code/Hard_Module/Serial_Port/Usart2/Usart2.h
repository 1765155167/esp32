#ifndef _USART2_H
#define _USART2_H

#include "global.h"
#include "stdio.h"


#define USART2_BUFFER_TX_SIZE                30        //USART1 DMA�����С
#define USART2_BUFFER_RX_SIZE               1050       //USART1 DMA�����С

#define TEST_Printf	1//��ӡ�������ݣ�1��/��ֹ��ӡ��0��

#if   1
    #define  Debug_printf(fmt,args...)  printf (fmt,##args)
#else
    #define  Debug_printf(fmt,args...)
#endif


//usart1�����ṹ��
typedef struct USART2_DATA
{
  unsigned char  usart2_TX_buffer[USART2_BUFFER_TX_SIZE];//usart2�ķ��ͻ���
  volatile unsigned char usart2_tx_DMA_free;//USART,����2dma���Ϳ��б��
  unsigned char  usart2_RX_buffer[USART2_BUFFER_RX_SIZE];//usart2�Ľ��ջ���
  volatile unsigned char usart2_rx_DMA_free;//USART,����2dma���տ��б��
  volatile unsigned int  usart2_rx_DMA_count;//USART,����2dma�������ݳ���
  volatile unsigned int  usart2_tx_DMA_count;
  volatile unsigned char flag_delay;//�����ж��Ƿ���Ҫ��ʱ
} Usart2_Data_Struct;

extern Usart2_Data_Struct  Usart2_Data;//����2���ݽṹ��

void USART2_Configuration(void);//uart2���ڳ�ʼ��
void USART2_SendDMA(unsigned char* buf,unsigned short int length);//DMA��ʽ�����ַ���
void USART2_Rev_DMA_Flag_Clear(void);//DMA�����ַ�����־����������ٴν�������
void Usart2_Interrupt(void);//����1�жϺ���	
void USART2_Re_Data_Deal(void);//����1�������ݴ���
void USART2_TX_Data(unsigned char* buf,unsigned short int length);//����2���ݷ���

#endif

