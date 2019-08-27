#ifndef __Usart_Magagement_H
#define __Usart_Magagement_H 		

#include "global.h"

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);//DMA串口发送完成中断回调函数

#endif
