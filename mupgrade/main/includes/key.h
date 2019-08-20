#ifndef _KEY_H__
#define _KEY_H__

#include "config.h"
#include "key.h"
#include "led.h"
#include "tcp.h"
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"

typedef enum {
    KEY1_SHORT_ONCE = 0x0,    /**/
    KEY2_SHORT_ONCE,
    KEY3_SHORT_ONCE,
    KEY4_SHORT_ONCE,
    KEY4_LONG,
} key_flag;

typedef enum {
	RisingEdge = 0x0,/*上升沿*/
	FallingEdge/*下降沿*/
} key_edge;

typedef struct key
{
	int32_t    io_num;              /* io引脚号 */
	int32_t    status_led;          /*状态灯引脚号*/
	BaseType_t status_led_flag;     /*状态灯状态标志*/
	int32_t    relay_led;           /*背光灯*/
	BaseType_t relay_led_flag;      /*背光灯亮灭标志*/    
	BaseType_t press_key;           /* 按键状态 按下*/
	BaseType_t lift_key;            /* 按键状态 抬起*/
	BaseType_t time_start;          /* 消抖动定时器开始标志 */
	BaseType_t trigger;             /*　按键按下触发电平*/
	int64_t    tick;                /* 按键按下的时间 */
	// int8_t     keyclick;      /* 按键单击次数 */
}mykey_t;

mdf_err_t key_init(void);

#endif
