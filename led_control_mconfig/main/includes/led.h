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

typedef enum {
	ON_LED,/*开*/
	OFF_LED
}onoff_led;

void targiet_led(int id);/*翻转*/
void set_led_flag(int id,onoff_led flag);
mdf_err_t led_init(void);
size_t get_flag_info(char * data, int count);
#endif