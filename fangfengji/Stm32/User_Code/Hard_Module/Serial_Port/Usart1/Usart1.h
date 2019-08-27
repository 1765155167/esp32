#ifndef _USART1_H
#define _USART1_H

#include "global.h"


#define USART1_BUFFER_TX_SIZE                30        //USART1 DMA�����С
#define USART1_BUFFER_RX_SIZE               1050       //USART1 DMA�����С

#define Usart1_Delay HAL_Delay

//usart1�����ṹ��
typedef struct USART1_DATA
{
	unsigned char usart1_TX_buffer[USART1_BUFFER_TX_SIZE];//usart1�ķ��ͻ���
	volatile unsigned char usart1_tx_DMA_free;//USART,����1dma���Ϳ��б��
	unsigned char usart1_RX_buffer[USART1_BUFFER_RX_SIZE];//usart1�Ľ��ջ���
	volatile unsigned char usart1_rx_DMA_free;//USART,����1dma���տ��б��
	volatile unsigned int  usart1_rx_DMA_count;//USART,����1dma�������ݳ���
        volatile unsigned int  usart1_tx_DMA_count;
        volatile unsigned char flag_delay;//�����ж��Ƿ���Ҫ��ʱ
} Usart1_Data_Struct;

extern Usart1_Data_Struct  Usart1_Data;//����1���ݽṹ��

void USART1_Configuration(void);//uart1���ڳ�ʼ��
void USART1_SendDMA(unsigned char* buf,unsigned short int length);//DMA��ʽ�����ַ���
void USART1_Rev_DMA_Flag_Clear(void);//DMA�����ַ�����־����������ٴν�������
void Usart1_Interrupt(void);//����1�жϺ���	
void USART1_Re_Data_Deal(void);//����1�������ݴ���
void Scopy(unsigned char str1[],unsigned char str2[],unsigned int length);//��str2���Ƶ�str1
unsigned char judeg_str_equal(unsigned char* str1,unsigned char* str2,unsigned char length);//�жϵ�2���ַ���ǰn���ַ��Ƿ����
void USART1_TX_Data(unsigned char* buf,unsigned short int length);//����1���ݷ���

#endif
