#ifndef _moter_H_
#define _moter_H_

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "moter_nvs.h"
#include "key.h"
#include "led.h"

/*moter cfg*/
#define MOTER1_FORWARD_IO 17//电机1正转 io口
#define MOTER1_REVERSE_IO 5//电机1反转 io口
#define MOTER2_FORWARD_IO 18//电机2正转 io口
#define MOTER2_REVERSE_IO 19//电机2反转 io口
#define UP_INFO_TIMER 120 //定时上传时间间隔 s

typedef struct moter_stu {
	char * Typ;			//设备类型
	int32_t NTemp;		//实时温度
	int32_t OpenPer;	//风口当前打开程度千分比
	char * ConSta;		//当前控制模式 `auto` 自动模式 `manual` 手动模式
	char * MoSta;		//电机状态
}moter_stu;

typedef struct moter_args{
	int32_t AlarmTempMax;	//报警高温
	int32_t AlarmTempMin;	//报警低温
	int32_t SetTempMax;	//设定控制温度上限
	int32_t SetTempMin;	//设定控制温度下限
	uint32_t TotalTime;		//风口完整开启或关闭一次所需时间，单位s
}moter_args;

mdf_err_t manual_moter(char * data, uint8_t id);/*手动控制*/
mdf_err_t set_args_info(char * data, uint8_t id);/*参数配置*/
mdf_err_t get_json_info(char * json_info,int id);
mdf_err_t moter_change_mode(int io);/*改变放风机模式*/
mdf_err_t moter_set_mode(char * data, uint8_t id);/*设置放风机模式*/
mdf_err_t moter_openAdjust(char * data, uint8_t id);/*风口校准*/
mdf_err_t moter_tempAdjust(char * data, uint8_t id);/*温度校准*/
mdf_err_t moter_forward(int io);
mdf_err_t moter_reverse(int io);
mdf_err_t moter_stop(int io);
mdf_err_t moter_init(void);

#endif