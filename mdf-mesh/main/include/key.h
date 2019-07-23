#ifndef _KEY_H__
#define _KEY_H__
#include "led.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#define KEY1_GPIO 36 /* key　引脚号 */
#define KEY2_GPIO 39 /* key　引脚号 */
#define KEY3_GPIO 34 /* key　引脚号 */
#define KEY4_GPIO 35 /* key　引脚号 */

#define longTime  1000  /* 长按最短时间 ms */
#define keyxd 15 /* 按键消抖时长 ms */

typedef enum {
    KEY1_SHORT_ONCE = 0x0,    /**/
    KEY2_SHORT_ONCE,
    KEY3_SHORT_ONCE,
    KEY4_SHORT_ONCE,
    KEY4_LONG,
} key_flag;

typedef struct key
{
	int32_t    io_num;              /* io引脚号 */
	BaseType_t press_key;           /* 按键状态 按下*/
	BaseType_t lift_key;            /* 按键状态 抬起*/
	BaseType_t time_start;          /* 消抖动定时器开始标志 */
	int64_t    tick;                /* 按键按下的时间 */
	int8_t     keyclick;      /* 按键单击次数 */
}mykey_t;

mdf_err_t key_init(void);

#endif
