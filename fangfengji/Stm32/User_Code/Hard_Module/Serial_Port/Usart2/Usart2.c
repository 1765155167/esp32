#include "Usart2.h"


/****************************˽�к궨��*******************************/

/****************************˽�к�������*****************************/

/****************************˽��ȫ�ֱ�������*************************/

/****************************����ȫ�ֱ�������*************************/
Usart2_Data_Struct  Usart2_Data;//����2���ݽṹ��

/****************************˽�к���*********************************/

/****************************���к���*********************************/
/****************************************************************************
* ��    �ƣ�void USART2_uConfiguration
* ��    �ܣ�uart2���ڳ�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART2_Configuration(void)
{
  USART2_Rev_DMA_Flag_Clear();//������
  Usart2_Data.usart2_rx_DMA_free = 0;//���������һ������	
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//ʹ�ܴ���2�����ж�
}

/****************************************************************************
* ��    �ƣ�void USART2_SendDMA
* ��    �ܣ�DMA��ʽ�����ַ���
* ��ڲ�����unsigned char* buf:�����͵��ַ���,unsigned short int length�������͵��ַ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART2_SendDMA(unsigned char* buf,unsigned short int length)
{
  u8 i = 0;
  while(Usart2_Data.usart2_tx_DMA_free == 1)//�ȴ�DMA����
  {
    i++;
    HAL_Delay(10);
    if(i > 10)
    {
      return;
    }
  }
  
  HAL_UART_Transmit_DMA(&huart2, buf, length);  
  Usart2_Data.usart2_tx_DMA_free = 1;//USART,����2dma���Ϳ��б����1
}

/****************************************************************************
* ��    �ƣ�USART2_TX_Data
* ��    �ܣ�����2���ݷ���
* ��ڲ�����unsigned char* buf:�����͵��ַ���,unsigned short int length�������͵��ַ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART2_TX_Data(unsigned char* buf,unsigned short int length)
{
    u8 i = 0;
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    USART2_SendDMA(buf,length);
    while(Usart2_Data.usart2_tx_DMA_free == 1)//�ȴ�DMA����
    {
      i++;
      HAL_Delay(1);
      if(i > 10)
      {
        break;
      }
    }
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//ʹ�ܴ���2�����ж�
    USART2_Rev_DMA_Flag_Clear();//���������һ������    
    Usart2_Data.usart2_rx_DMA_free = 0;//���������һ������
}

/****************************************************************************
* ��    �ƣ� USART2_Rev_DMA_Flag_Clear(void)
* ��    �ܣ�DMA�����ַ�����־����������ٴν�������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART2_Rev_DMA_Flag_Clear(void)
{
  HAL_UART_DMAStop(&huart2);
  HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);//ʹ��dmaͨ��
  __HAL_UART_CLEAR_PEFLAG(&huart2);//���еı�Ƕ��������ֹ����
  __HAL_UART_CLEAR_FEFLAG(&huart2);
  __HAL_UART_CLEAR_NEFLAG(&huart2);
  __HAL_UART_CLEAR_OREFLAG(&huart2);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);  
}

/****************************************************************************
* ��    �ƣ�void Usart2_Interrupt(void)   
* ��    �ܣ�����2�жϺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Usart2_Interrupt(void)  
{  	
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE) != RESET)
  {
    if(Usart2_Data.flag_delay == 1)//�����Ҫ��ʱ�����ڽ��һЩ���ͷ��ֶη�����Ϣ���������
    {
      HAL_Delay(10);
      Usart2_Data.flag_delay = 0;
    }
    
    
    if(Usart2_Data.usart2_rx_DMA_free == 0)//�ϴν��յ�ָ���Ѿ�������
    {
      HAL_UART_DMAStop(&huart2);
      Usart2_Data.usart2_rx_DMA_count = USART2_BUFFER_RX_SIZE - huart2.hdmarx->Instance->CNDTR;//���յ��ĳ���
      
      if( (Usart2_Data.usart2_rx_DMA_count > 0) )//������յ��ĳ��ȴ���0
      {
        Usart2_Data.usart2_rx_DMA_free = 1;//ָ�������
      }
      else//����
      {
        HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);
      }
    }
    else
    {
      HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);
    }
  }
  __HAL_UART_CLEAR_PEFLAG(&huart2);//���еı�Ƕ��������ֹ����
  __HAL_UART_CLEAR_FEFLAG(&huart2);
  __HAL_UART_CLEAR_NEFLAG(&huart2);
  __HAL_UART_CLEAR_OREFLAG(&huart2);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);  
}	

/****************************************************************************
* ��    �ƣ�USART2_Re_Data_Deal
* ��    �ܣ�����2�������ݴ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void USART2_Re_Data_Deal(void)
{
  int i = 0;
  u8* p = 0;
  
  if( Usart2_Data.usart2_rx_DMA_free == 1)//�յ���������
  {   
    Scopy(Usart1_Data.usart1_RX_buffer,Usart2_Data.usart2_RX_buffer,Usart2_Data.usart2_rx_DMA_count);
    Usart1_Data.usart1_tx_DMA_count = Usart2_Data.usart2_rx_DMA_count;
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    
    
    if(Usart1_Data.usart1_RX_buffer[0] != 0xfe)
    {
      HAL_UART_DMAStop(&huart1);
      __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
      USART1_SendDMA(Usart1_Data.usart1_RX_buffer,Usart1_Data.usart1_tx_DMA_count); 
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
      Usart1_Data.usart1_rx_DMA_free = 0;
    }
    else
    {
      p = FrameDeal(Usart1_Data.usart1_RX_buffer);//����֡����
      USART2_SendDMA(p,GetFrameLen(p)); 
      while(Usart2_Data.usart2_tx_DMA_free == 1)//�ȴ�DMA����
      {
        i++;
        HAL_Delay(1);
        if(i > 10)
        {
          break;
        }
      }
    }
    
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//ʹ�ܴ���2�����ж�
    USART2_Rev_DMA_Flag_Clear();//���������һ������    
    Usart2_Data.usart2_rx_DMA_free = 0;//���������һ������    
  }
  else
  {
    
  }
}

