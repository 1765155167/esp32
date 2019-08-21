#ifndef _CONFIG_H_
#define _CONFIG_H_

/*状态灯*/
#define RELAY1 17
#define RELAY2 5
#define RELAY3 18
#define RELAY4 19
/*背光灯*/
#define RELAY1_LED 13
#define RELAY2_LED 21
#define RELAY3_LED 22
#define RELAY4_LED 23

#define KEY1_GPIO 33 /* key　引脚号 */
#define KEY2_GPIO 32 /* key　引脚号 */
#define KEY3_GPIO 25 /* key　引脚号 */
#define KEY4_GPIO 26 /* key　引脚号 */
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
