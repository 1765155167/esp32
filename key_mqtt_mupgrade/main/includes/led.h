#ifndef _LED_H_
#define _LED_H_

#include "config.h"
#include "mwifi.h"
#include "mdf_common.h"


void set_led(int led);/*点亮*/
void unset_led(int led);/*熄灭*/
void targiet_led(int led);/*翻转*/
mdf_err_t led_init(void);

#endif