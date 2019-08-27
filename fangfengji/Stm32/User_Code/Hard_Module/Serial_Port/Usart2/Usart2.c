#include "Usart2.h"


/****************************私有宏定义*******************************/

/****************************私有函数声明*****************************/

/****************************私有全局变量定义*************************/

/****************************公有全局变量定义*************************/
Usart2_Data_Struct  Usart2_Data;//串口2数据结构体

/****************************私有函数*********************************/

/****************************公有函数*********************************/
/****************************************************************************
* 名    称：void USART2_uConfiguration
* 功    能：uart2串口初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_Configuration(void)
{
  USART2_Rev_DMA_Flag_Clear();//标记清空
  Usart2_Data.usart2_rx_DMA_free = 0;//允许接收下一个命令	
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//使能串口2空闲中断
}

/****************************************************************************
* 名    称：void USART2_SendDMA
* 功    能：DMA方式发送字符串
* 入口参数：unsigned char* buf:待发送的字符串,unsigned short int length：待发送的字符串长度
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_SendDMA(unsigned char* buf,unsigned short int length)
{
  u8 i = 0;
  while(Usart2_Data.usart2_tx_DMA_free == 1)//等待DMA空闲
  {
    i++;
    HAL_Delay(10);
    if(i > 10)
    {
      return;
    }
  }
  
  HAL_UART_Transmit_DMA(&huart2, buf, length);  
  Usart2_Data.usart2_tx_DMA_free = 1;//USART,串口2dma发送空闲标记置1
}

/****************************************************************************
* 名    称：USART2_TX_Data
* 功    能：串口2数据发送
* 入口参数：unsigned char* buf:待发送的字符串,unsigned short int length：待发送的字符串长度
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_TX_Data(unsigned char* buf,unsigned short int length)
{
    u8 i = 0;
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    USART2_SendDMA(buf,length);
    while(Usart2_Data.usart2_tx_DMA_free == 1)//等待DMA空闲
    {
      i++;
      HAL_Delay(1);
      if(i > 10)
      {
        break;
      }
    }
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//使能串口2空闲中断
    USART2_Rev_DMA_Flag_Clear();//允许接收下一条命令    
    Usart2_Data.usart2_rx_DMA_free = 0;//允许接收下一个命令
}

/****************************************************************************
* 名    称： USART2_Rev_DMA_Flag_Clear(void)
* 功    能：DMA发送字符串标志清楚，允许再次接收命令
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_Rev_DMA_Flag_Clear(void)
{
  HAL_UART_DMAStop(&huart2);
  HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);//使能dma通道
  __HAL_UART_CLEAR_PEFLAG(&huart2);//所有的标记都清除，防止错误
  __HAL_UART_CLEAR_FEFLAG(&huart2);
  __HAL_UART_CLEAR_NEFLAG(&huart2);
  __HAL_UART_CLEAR_OREFLAG(&huart2);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);  
}

/****************************************************************************
* 名    称：void Usart2_Interrupt(void)   
* 功    能：串口2中断函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Usart2_Interrupt(void)  
{  	
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE) != RESET)
  {
    if(Usart2_Data.flag_delay == 1)//如果需要延时，用于解决一些发送方分段发送消息的特殊情况
    {
      HAL_Delay(10);
      Usart2_Data.flag_delay = 0;
    }
    
    
    if(Usart2_Data.usart2_rx_DMA_free == 0)//上次接收的指令已经处理完
    {
      HAL_UART_DMAStop(&huart2);
      Usart2_Data.usart2_rx_DMA_count = USART2_BUFFER_RX_SIZE - huart2.hdmarx->Instance->CNDTR;//接收到的长度
      
      if( (Usart2_Data.usart2_rx_DMA_count > 0) )//如果接收到的长度大于0
      {
        Usart2_Data.usart2_rx_DMA_free = 1;//指令待处理
      }
      else//误判
      {
        HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);
      }
    }
    else
    {
      HAL_UART_Receive_DMA(&huart2,Usart2_Data.usart2_RX_buffer,USART2_BUFFER_RX_SIZE);
    }
  }
  __HAL_UART_CLEAR_PEFLAG(&huart2);//所有的标记都清除，防止错误
  __HAL_UART_CLEAR_FEFLAG(&huart2);
  __HAL_UART_CLEAR_NEFLAG(&huart2);
  __HAL_UART_CLEAR_OREFLAG(&huart2);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);  
}	

/****************************************************************************
* 名    称：USART2_Re_Data_Deal
* 功    能：串口2接收数据处理
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART2_Re_Data_Deal(void)
{
  int i = 0;
  u8* p = 0;
  
  if( Usart2_Data.usart2_rx_DMA_free == 1)//收到串口数据
  {   
    Scopy(Usart1_Data.usart1_RX_buffer,Usart2_Data.usart2_RX_buffer,Usart2_Data.usart2_rx_DMA_count);
    Usart1_Data.usart1_tx_DMA_count = Usart2_Data.usart2_rx_DMA_count;
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    
    
    if(Usart1_Data.usart1_RX_buffer[0] != 0xfe)
    {
      HAL_UART_DMAStop(&huart1);
      __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
      USART1_SendDMA(Usart1_Data.usart1_RX_buffer,Usart1_Data.usart1_tx_DMA_count); 
      while(Usart1_Data.usart1_tx_DMA_free == 1)//等待DMA空闲
      {
        i++;
        HAL_Delay(1);
        if(i > 10)
        {
          break;
        }
      }
      __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//使能串口1空闲中断
      USART1_Rev_DMA_Flag_Clear();//允许接收下一条命令  
      Usart1_Data.usart1_rx_DMA_free = 0;
    }
    else
    {
      p = FrameDeal(Usart1_Data.usart1_RX_buffer);//数据帧处理
      USART2_SendDMA(p,GetFrameLen(p)); 
      while(Usart2_Data.usart2_tx_DMA_free == 1)//等待DMA空闲
      {
        i++;
        HAL_Delay(1);
        if(i > 10)
        {
          break;
        }
      }
    }
    
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//使能串口2空闲中断
    USART2_Rev_DMA_Flag_Clear();//允许接收下一条命令    
    Usart2_Data.usart2_rx_DMA_free = 0;//允许接收下一个命令    
  }
  else
  {
    
  }
}

