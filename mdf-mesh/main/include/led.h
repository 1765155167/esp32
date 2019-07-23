#ifndef _LED_H__
#define _LED_H__

#include "mdf_err.h"
#include "moter.h"

#define LED1_GPIO 25 /* key　引脚号 */
#define LED2_GPIO 26 /* key　引脚号 */
#define LED3_GPIO 27 /* key　引脚号 */

mdf_err_t led_init(void);
void key_led_press(int key);/* 按键处理 */
void json_led_press(char * data);/* json 信息处理 */

#endif
