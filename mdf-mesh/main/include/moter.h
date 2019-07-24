#ifndef _moter_H_
#define _moter_H_

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "key.h"
#include "led.h"

/*moter cfg*/
#define MOTER1_FORWARD_IO 17//电机1正转 io口
#define MOTER1_REVERSE_IO 5//电机1反转 io口
#define MOTER2_FORWARD_IO 18//电机2正转 io口
#define MOTER2_REVERSE_IO 19//电机2反转 io口
#define ONEMIN_OPENPER 60 /*１分钟开60%*/
typedef struct moter_stu {
	char * Typ;			//设备类型
	uint8_t NTemp;		//实时温度
	uint8_t OpenPer;	//风口当前打开程度百分比
	char * ConSta;		//当前控制模式 `auto` 自动模式 `manual` 手动模式
	char * MoSta;		//电机状态
}moter_stu;

mdf_err_t get_json_info(char * json_info,int id);
mdf_err_t moter_set_mode(int io);
mdf_err_t moter_forward(int io);
mdf_err_t moter_reverse(int io);
mdf_err_t moter_stop(int io);
mdf_err_t moter_init(void);
#endif