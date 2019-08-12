#ifndef TEMP_MEASURE_H
#define TEMP_MEASURE_H

#include "config.h"
#include <driver/adc.h>
#include "temp_table.h"
#include "kalman_math.h"
#include "esp_adc_cal.h"
#include "moter.h"

/*temp cfg*/
#define TP_MEASURE_PEROID_MS 1000 //温度测量间隔
#define TP_Q 0.005 //温度滤波参数
#define TP_R 1//温度滤波参数

typedef struct temp_info_t{
    float kom;
    float kom_offset;
	float TempCalOff;/*温度校准偏移*/
    int channel;/*　管道 */
    kalman_t *kalman;/* 卡尔曼滤波 */
}temp_info_t;

temp_info_t *build_temp_info(int adc_channel);
float get_temp(temp_info_t *info);/*读取温度*/
float calcu_kom_offset(temp_info_t *info, int true_temp);/*获取true_temp对应的电阻值*/
void print_temp_info(temp_info_t *temp_info);
void tempCal(temp_info_t *info,float temp);/*温度校准*/
#endif