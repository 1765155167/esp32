#ifndef _LED_H__
#define _LED_H__

#include "mdf_err.h"
#include "moter.h"
#include "config.h"

enum LED_STATUS {
	MESH_CONNECTION_ERROR = 0,
	HIGH_TEMP,
	LOW_TEMP
};

mdf_err_t led_init(void);
void key_led_press(int key);/* 按键处理 */
void json_led_press(char * data);/* json 信息处理 */
mdf_err_t information_Upload(char * json_info);/*上传信息*/
mdf_err_t up_alarm_temp_info(int id);/*上传温度报警信息*/
void led_status_set(int status);
void led_status_unset(int status);

#endif
