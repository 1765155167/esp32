#ifndef _LED_H_
#define _LED_H_

#include "config.h"
#include "mwifi.h"
#include "mdf_common.h"

struct led
{
	int32_t    io_num; /* io引脚号 */
	BaseType_t flag;   /* 是否低电平点亮*/
};

struct ledflag {
	struct led    light;           /* io引脚号 */
	struct led    backlight;       /*背光灯引脚号*/
	BaseType_t    led_flage;       /*灯的状态*/
};




#define LED_FLAG_CHANGE BIT0

void set_led(struct led led);/*点亮*/
void unset_led(struct led led);/*熄灭*/
void targiet_led(int led);/*翻转*/
mdf_err_t led_init(void);

#endif