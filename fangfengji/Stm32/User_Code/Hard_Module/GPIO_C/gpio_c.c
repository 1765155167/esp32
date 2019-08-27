#include "gpio_c.h"


/****************************************************************************
* 名    称：void Gpio_Pin_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
* 功    能：STM32F0系列端口置位函数
* 入口参数：unsigned char GPIO_X:需要操作的GPIO组,unsigned char GPIO_PIN_X：需要操作的GPIO口
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Gpio_Pin_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
{	
  switch(GPIO_X)
  {
    case 'A':
                HAL_GPIO_WritePin(GPIOA,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;
    case 'B':
                HAL_GPIO_WritePin(GPIOB,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;
    case 'C':
                HAL_GPIO_WritePin(GPIOC,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    case 'D':
                HAL_GPIO_WritePin(GPIOD,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    case 'F':
                HAL_GPIO_WritePin(GPIOF,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_SET);
                break;	
    default:   break;
  }	
}

/****************************************************************************
* 名    称：void Gpio_Pin_Reset(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
* 功    能：STM32F0系列端口复位函数
* 入口参数：unsigned char GPIO_X:需要操作的GPIO组,unsigned char GPIO_PIN_X：需要操作的GPIO口
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Gpio_Pin_Reset(unsigned char GPIO_X,unsigned char GPIO_PIN_X)
{	
  switch(GPIO_X)
  {
    case 'A':
                HAL_GPIO_WritePin(GPIOA,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;
    case 'B':
                HAL_GPIO_WritePin(GPIOB,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;
    case 'C':
                HAL_GPIO_WritePin(GPIOC,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    case 'D':
                HAL_GPIO_WritePin(GPIOD,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    case 'F':
                HAL_GPIO_WritePin(GPIOF,0x0000|(1u<<GPIO_PIN_X),GPIO_PIN_RESET);
                break;	
    default:   break;
  }	
}

/****************************************************************************
* 名    称：void Gpio_Pin_Reset_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X,unsigned char bit)
* 功    能：STM32F0系列端口复位,置位函数
* 入口参数：unsigned char GPIO_X:需要操作的GPIO组,unsigned char GPIO_PIN_X：需要操作的GPIO口，unsigned char bit：1置位，0复位
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void Gpio_Pin_Reset_Set(unsigned char GPIO_X,unsigned char GPIO_PIN_X,unsigned char bit)
{	
  if(bit == 0)
  {
    Gpio_Pin_Reset(GPIO_X,GPIO_PIN_X);
  }
  else  
  {
    Gpio_Pin_Set(GPIO_X,GPIO_PIN_X);
  }
}


