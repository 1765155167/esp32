#ifndef _KEY_H__
#define _KEY_H__

#define KEY_GPIO 4 /* key　引脚号 */
#define clickTime 120  /* 按键多击间隔 120ms */
#define longTime  1000  /* 长按最短时间 1000ms */
void key_process(void *arg);/* 按键扫描任务函数 */

#endif
