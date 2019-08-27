#include "Usart1.h"

/****************************˽�к궨��*******************************/

/****************************˽�к�������*****************************/

/****************************˽��ȫ�ֱ�������*************************/

/****************************����ȫ�ֱ�������*************************/

Usart1_Data_Struct  Usart1_Data;//����1���ݽṹ��

/****************************˽�к���*********************************/

/****************************���к���*********************************/

/****************************************************************************
* ��    �ƣ�void USART1_uConfiguration
* ��    �ܣ�uart1���ڳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART1_Configuration(void)
{
 
  USART1_Rev_DMA_Flag_Clear();//������
  Usart1_Data.usart1_rx_DMA_free = 0;//���������һ������	
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//ʹ�ܴ���1�����ж�
}

/****************************************************************************
* ��    �ƣ�void USART1_SendDMA
* ��    �ܣ�DMA��ʽ�����ַ���
* ��ڲ�����unsigned char* buf:�����͵��ַ���,unsigned short int length�������͵��ַ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART1_SendDMA(unsigned char* buf,unsigned short int length)
{
  u8 i = 0;
  while(Usart1_Data.usart1_tx_DMA_free == 1)//�ȴ�DMA����
  {
    i++;
    HAL_Delay(10);
    if(i > 10)
    {
      return;
    }
  }
  
  HAL_UART_Transmit_DMA(&huart1, buf, length);  
  Usart1_Data.usart1_tx_DMA_free = 1;//USART,����1dma���Ϳ��б����1
}

/****************************************************************************
* ��    �ƣ�USART1_TX_Data
* ��    �ܣ�����1���ݷ���
* ��ڲ�����unsigned char* buf:�����͵��ַ���,unsigned short int length�������͵��ַ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART1_TX_Data(unsigned char* buf,unsigned short int length)
{
    u8 i = 0;
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
    USART1_SendDMA(buf,length);
    while(Usart1_Data.usart1_tx_DMA_free == 1)//�ȴ�DMA����
    {
      i++;
      HAL_Delay(1);
      if(i > 10)
      {
        break;
      }
    }
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//ʹ�ܴ���1�����ж�
    USART1_Rev_DMA_Flag_Clear();//���������һ������    
    Usart1_Data.usart1_rx_DMA_free = 0;//���������һ������
}

/****************************************************************************
* ��    �ƣ� USART1_Rev_DMA_Flag_Clear(void)
* ��    �ܣ�DMA�����ַ�����־����������ٴν�������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART1_Rev_DMA_Flag_Clear(void)
{
  HAL_UART_DMAStop(&huart1);
  HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);//ʹ��dmaͨ��
  __HAL_UART_CLEAR_PEFLAG(&huart1);//���еı�Ƕ��������ֹ����
  __HAL_UART_CLEAR_FEFLAG(&huart1);
  __HAL_UART_CLEAR_NEFLAG(&huart1);
  __HAL_UART_CLEAR_OREFLAG(&huart1);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);  
}

/****************************************************************************
* ��    �ƣ�void Usart1_Interrupt(void)   
* ��    �ܣ�����1�жϺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Usart1_Interrupt(void)  
{  	
  if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE) != RESET)
  {
    if(Usart1_Data.flag_delay == 1)//�����Ҫ��ʱ�����ڽ��һЩ���ͷ��ֶη�����Ϣ���������
    {
      HAL_Delay(10);
      Usart1_Data.flag_delay = 0;
    }
    
    if(Usart1_Data.usart1_rx_DMA_free == 0)//�ϴν��յ�ָ���Ѿ�������
    {
      HAL_UART_DMAStop(&huart1);
      Usart1_Data.usart1_rx_DMA_count = USART1_BUFFER_RX_SIZE - huart1.hdmarx->Instance->CNDTR;//���յ��ĳ���
      if( (Usart1_Data.usart1_rx_DMA_count > 0) )//������յ��ĳ��ȴ���0
      {
        Usart1_Data.usart1_rx_DMA_free = 1;//ָ�������
      }
      else//����
      {
        HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);
      }
    }
    else
    {
      HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);
    }
  }
  __HAL_UART_CLEAR_PEFLAG(&huart1);//���еı�Ƕ��������ֹ����
  __HAL_UART_CLEAR_FEFLAG(&huart1);
  __HAL_UART_CLEAR_NEFLAG(&huart1);
  __HAL_UART_CLEAR_OREFLAG(&huart1);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);  
}	

/****************************************************************************
* ��    �ƣ�USART1_Re_Data_Deal
* ��    �ܣ�����1�������ݴ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART1_Re_Data_Deal(void)
{
  int i = 0;
  if( Usart1_Data.usart1_rx_DMA_free == 1)//�յ���������
  {
    Scopy(Usart2_Data.usart2_RX_buffer,Usart1_Data.usart1_RX_buffer,Usart1_Data.usart1_rx_DMA_count);
    Usart2_Data.usart2_tx_DMA_count = Usart1_Data.usart1_rx_DMA_count;
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
   
  
    
    //HAL_UART_DMAStop(&huart2);
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    USART2_SendDMA(Usart2_Data.usart2_RX_buffer,Usart2_Data.usart2_tx_DMA_count);
    while(Usart2_Data.usart2_tx_DMA_free == 1)//�ȴ�DMA����
    {
      i++;
      HAL_Delay(1);
      if(i > 10)
      {
        break;
      }
    }
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//ʹ�ܴ���1�����ж�
    USART2_Rev_DMA_Flag_Clear();//���������һ������    
    Usart2_Data.usart2_rx_DMA_free = 0;//���������һ������
    
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//ʹ�ܴ���1�����ж�
    USART1_Rev_DMA_Flag_Clear();//���������һ������  
    Usart1_Data.usart1_rx_DMA_free = 0;//���������һ������
  }
}

/****************************************************************************
* ��    �ƣ�Scopy
* ��    �ܣ���str2���Ƶ�str1
* ��ڲ�����unsigned char str1[],unsigned char str2[],unsigned int length
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Scopy(unsigned char str1[],unsigned char str2[],unsigned int length)
{
  unsigned int i = 0;
  for(i = 0;i < length;i++)
  {
    str1[i] = str2[i];
  }
}

/*********************************************************************************************************
*	�� �� ��: fputc
*	����˵��: �ض���putc��������������ʹ��printf�����Ӵ���2��ӡ���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************/
int fputc(int ch,FILE *f)
{
  uint8_t temp[1]={ch};
  HAL_UART_Transmit(&huart2,temp,1,2);
  return(ch);
}

