#ifndef _KEY_H__
#define _KEY_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/portmacro.h"

#define KEY1_GPIO 36 /* key　引脚号 */
#define KEY2_GPIO 39 /* key　引脚号 */
#define KEY3_GPIO 34 /* key　引脚号 */
#define KEY4_GPIO 35 /* key　引脚号 */

#define clickTime 150   /* 按键多击间隔 ms */
#define longTime  800  /* 长按最短时间 ms */
void key_process(void *arg);/* 按键扫描任务函数 */

typedef struct key
{
	int32_t    io_num;              /* io引脚号 */
	BaseType_t press_key;           /* 按键状态 按下*/
	BaseType_t lift_key;            /* 按键状态 抬起*/
	BaseType_t time_start;          /* 消抖动定时器开始标志 */
	int64_t    tick;                /* 按键按下的时间 */
	int8_t     keyclick;      /* 按键单击次数 */
}mykey_t;

#endif
