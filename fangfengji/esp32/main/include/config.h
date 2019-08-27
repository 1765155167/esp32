#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "esp_adc_cal.h"

/*moter cfg　放风机继电器引脚*/
#define MOTER1_FORWARD_IO 17 //电机1正转 io口
#define MOTER1_REVERSE_IO 5  //电机1反转 io口
#define MOTER2_FORWARD_IO 18 //电机2正转 io口
#define MOTER2_REVERSE_IO 19 //电机2反转 io口

/*timer*/
#define UP_INFO_TIMER 50    //定时上传时间间隔 s
#define GET_TEMP_INFO 5      //计算温度时间间隔 s
#define TEMP_ALARM_TIME 5    //温度报警时间间隔 s
#define AUTO_CTRL_TIME 20    //自动控制延迟时间 s

/*key按键*/
#define KEY1_GPIO 36 /* key　引脚号 */
#define KEY2_GPIO 39 /* key　引脚号 */
#define KEY3_GPIO 34 /* key　引脚号 */
#define KEY4_GPIO 35 /* key　引脚号 */
#define longTime  1000  /* 长按最短时间 ms */
#define keypresstime 15 /* 按键消抖时长 ms */
#define keylifttime 15 /* 按键消抖时长 ms */

/*状态灯*/
#define LED1_GPIO 25 /*　MESH网络链接错误 */
#define LED2_GPIO 26 /* 高温报警 */
#define LED3_GPIO 27 /* 低温报警 */

/*ntc　热敏电阻*/
#define ADC_CH1 ADC1_CHANNEL_4 //温度测量通道1使用的adc/*!< ADC1 channel 4 is GPIO32 */
#define ADC_CH2 ADC1_CHANNEL_5 //温度测量通道2使用的adc/*!< ADC1 channel 5 is GPIO33 */

#endif