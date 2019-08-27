#ifndef _CONFIG_H_
#define _CONFIG_H_

/*开关*/
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"

#define HIGH pdFALSE /*高电平点亮LED*/
#define LOW  pdTRUE  /*低电平点亮LED*/

/*状态灯 ４个低电平点亮*/
#define RELAY1 17
#define RELAY2 5
#define RELAY3 18
#define RELAY4 19
#define RELAY1_FLAG LOW
#define RELAY2_FLAG LOW
#define RELAY3_FLAG LOW
#define RELAY4_FLAG LOW
/*背光灯 4个高电平点亮*/
#define RELAY1_LED 33
#define RELAY2_LED 32
#define RELAY3_LED 25
#define RELAY4_LED 26
#define RELAY1_LED_FLAG LOW
#define RELAY2_LED_FLAG LOW
#define RELAY3_LED_FLAG LOW
#define RELAY4_LED_FLAG LOW

/*按键*/
#define KEY1_GPIO 36  /* key引脚号 */
#define KEY2_GPIO 39  /* key引脚号 */
#define KEY3_GPIO 34  /* key引脚号 */
#define KEY4_GPIO 35  /* key引脚号 */
#define RISINGEDGE 0  /*上升沿*/
#define FALLINGEDGE 1 /*下降沿*/
#define KEY1_GPIO_EDGE FALLINGEDGE /*按键1按下触发电平方式　下降沿*/
#define KEY2_GPIO_EDGE RISINGEDGE  /*按键2按下触发电平方式　上升沿*/
#define KEY3_GPIO_EDGE FALLINGEDGE /*按键3按下触发电平方式　下降沿*/
#define KEY4_GPIO_EDGE FALLINGEDGE /*按键4按下触发电平方式　下降沿*/
#define KEY_MAX   4  /* 按键个数 */
#define clickTime 150   /* 按键多击间隔 ms */
#define longTime  1000   /* 长按最短时间 ms */
#define DebounceTime 15 /*按键消抖时间 ms*/

/*初始化LED时用*/
#define LED0 4
#define LED1 RELAY1_LED
#define LED2 RELAY2_LED
#define LED3 RELAY3_LED
#define LED4 RELAY4_LED
#define LED5 RELAY1
#define LED6 RELAY2
#define LED7 RELAY3
#define LED8 RELAY4

#endif
