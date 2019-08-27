#include "Usart1.h"

/****************************私有宏定义*******************************/

/****************************私有函数声明*****************************/

/****************************私有全局变量定义*************************/

/****************************公有全局变量定义*************************/

Usart1_Data_Struct  Usart1_Data;//串口1数据结构体

/****************************私有函数*********************************/

/****************************公有函数*********************************/

/****************************************************************************
* 名    称：void USART1_uConfiguration
* 功    能：uart1串口初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART1_Configuration(void)
{
 
  USART1_Rev_DMA_Flag_Clear();//标记清空
  Usart1_Data.usart1_rx_DMA_free = 0;//允许接收下一个命令	
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//使能串口1空闲中断
}

/****************************************************************************
* 名    称：void USART1_SendDMA
* 功    能：DMA方式发送字符串
* 入口参数：unsigned char* buf:待发送的字符串,unsigned short int length：待发送的字符串长度
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART1_SendDMA(unsigned char* buf,unsigned short int length)
{
  u8 i = 0;
  while(Usart1_Data.usart1_tx_DMA_free == 1)//等待DMA空闲
  {
    i++;
    HAL_Delay(10);
    if(i > 10)
    {
      return;
    }
  }
  
  HAL_UART_Transmit_DMA(&huart1, buf, length);  
  Usart1_Data.usart1_tx_DMA_free = 1;//USART,串口1dma发送空闲标记置1
}

/****************************************************************************
* 名    称：USART1_TX_Data
* 功    能：串口1数据发送
* 入口参数：unsigned char* buf:待发送的字符串,unsigned short int length：待发送的字符串长度
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART1_TX_Data(unsigned char* buf,unsigned short int length)
{
    u8 i = 0;
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
    USART1_SendDMA(buf,length);
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
    Usart1_Data.usart1_rx_DMA_free = 0;//允许接收下一个命令
}

/****************************************************************************
* 名    称： USART1_Rev_DMA_Flag_Clear(void)
* 功    能：DMA发送字符串标志清楚，允许再次接收命令
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART1_Rev_DMA_Flag_Clear(void)
{
  HAL_UART_DMAStop(&huart1);
  HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);//使能dma通道
  __HAL_UART_CLEAR_PEFLAG(&huart1);//所有的标记都清楚，防止错误
  __HAL_UART_CLEAR_FEFLAG(&huart1);
  __HAL_UART_CLEAR_NEFLAG(&huart1);
  __HAL_UART_CLEAR_OREFLAG(&huart1);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);  
}

/****************************************************************************
* 名    称：void Usart1_Interrupt(void)   
* 功    能：串口1中断函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Usart1_Interrupt(void)  
{  	
  if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE) != RESET)
  {
    if(Usart1_Data.flag_delay == 1)//如果需要延时，用于解决一些发送方分段发送消息的特殊情况
    {
      HAL_Delay(10);
      Usart1_Data.flag_delay = 0;
    }
    
    if(Usart1_Data.usart1_rx_DMA_free == 0)//上次接收的指令已经处理完
    {
      HAL_UART_DMAStop(&huart1);
      Usart1_Data.usart1_rx_DMA_count = USART1_BUFFER_RX_SIZE - huart1.hdmarx->Instance->CNDTR;//接收到的长度
      if( (Usart1_Data.usart1_rx_DMA_count > 0) )//如果接收到的长度大于0
      {
        Usart1_Data.usart1_rx_DMA_free = 1;//指令待处理
      }
      else//误判
      {
        HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);
      }
    }
    else
    {
      HAL_UART_Receive_DMA(&huart1,Usart1_Data.usart1_RX_buffer,USART1_BUFFER_RX_SIZE);
    }
  }
  __HAL_UART_CLEAR_PEFLAG(&huart1);//所有的标记都清楚，防止错误
  __HAL_UART_CLEAR_FEFLAG(&huart1);
  __HAL_UART_CLEAR_NEFLAG(&huart1);
  __HAL_UART_CLEAR_OREFLAG(&huart1);   
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);  
}	

/****************************************************************************
* 名    称：USART1_Re_Data_Deal
* 功    能：串口1接收数据处理
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART1_Re_Data_Deal(void)
{
  int i = 0;
  if( Usart1_Data.usart1_rx_DMA_free == 1)//收到串口数据
  {
    Scopy(Usart2_Data.usart2_RX_buffer,Usart1_Data.usart1_RX_buffer,Usart1_Data.usart1_rx_DMA_count);
    Usart2_Data.usart2_tx_DMA_count = Usart1_Data.usart1_rx_DMA_count;
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
   
  
    
    //HAL_UART_DMAStop(&huart2);
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
    USART2_SendDMA(Usart2_Data.usart2_RX_buffer,Usart2_Data.usart2_tx_DMA_count);
    while(Usart2_Data.usart2_tx_DMA_free == 1)//等待DMA空闲
    {
      i++;
      HAL_Delay(1);
      if(i > 10)
      {
        break;
      }
    }
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);//使能串口1空闲中断
    USART2_Rev_DMA_Flag_Clear();//允许接收下一条命令    
    Usart2_Data.usart2_rx_DMA_free = 0;//允许接收下一个命令
    
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);//使能串口1空闲中断
    USART1_Rev_DMA_Flag_Clear();//允许接收下一条命令  
    Usart1_Data.usart1_rx_DMA_free = 0;//允许接收下一个命令
  }
}

/****************************************************************************
* 名    称：Scopy
* 功    能：将str2复制到str1
* 入口参数：unsigned char str1[],unsigned char str2[],unsigned int length
* 出口参数：无
* 说    明：
* 调用方法：无 
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
*	函 数 名: fputc
*	功能说明: 重定义putc函数，这样可以使用printf函数从串口2打印输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************/
int fputc(int ch,FILE *f)
{
  uint8_t temp[1]={ch};
  HAL_UART_Transmit(&huart2,temp,1,2);
  return(ch);
}

