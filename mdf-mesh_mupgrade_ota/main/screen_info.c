#include "screen_info.h"

static const char *TAG = "screen_info";

#define SCAN_TASK_SIZE 2048
#define SCAN_TASK_PRI ESP_TASK_MAIN_PRIO

#define SCAN_PEROID_MS 1000
#define MAX_INFO 2
#define TIME_OUT_MS 300

#define DEVICE_ID 0xfe

#define SCREEN_EN               0x102
#define SCREEN_ID               0x104
#define SCREEN_LENGTH           0x106
#define SCREEN_TEMP_NOW         0x108
#define SCREEN_TEMP_TREND       0x10a
#define SCREEN_TEMP_MAX         0x10c
#define SCREEN_TEMP_MIN         0x10e
#define SCREEN_MOTOR_STATUS     0x110
#define SCREEN_CONTROL_MODE     0x112

enum{
    SCREEN_CONTROL_AUTO  = 1,
    SCREEN_CONTROL_MANUAL = 2,
}_control_mode;

enum{
    SCREEN_MOTOR_FORWARD = 1,
    SCREEN_MOTOR_REVERSE = 2,
    SCREEN_MOTOR_STOP = 3,
}_motor_status;

enum{
    SCREEN_TEMP_TREND_DOWN = 1,
    SCREEN_TEMP_TREND_UP = 2,
}_trend_temp;

static screen_info_t *g_info[MAX_INFO]= {NULL};
static int offset = MAX_INFO - 1;
xSemaphoreHandle g_lock;

/*moter.c 设备参数信息与设备实时信息*/
extern moter_args moter_args1;
extern moter_args moter_args2;
extern moter_stu moter_flag1;
extern moter_stu moter_flag2;

static void screen_info_refresh(screen_info_t *info)
{
    MDF_LOGD("screen info refresh");
    
    int16_t temp;

    rtu_write_reg(DEVICE_ID, SCREEN_EN, info->en, TIME_OUT_MS);
    
    rtu_write_reg(DEVICE_ID, SCREEN_ID, info->id, TIME_OUT_MS);
	/*风口开度*/
    rtu_write_reg(DEVICE_ID, SCREEN_LENGTH, info->device->OpenPer, TIME_OUT_MS);

    temp = (int16_t )info->device->NTemp;
    rtu_write_reg(DEVICE_ID, SCREEN_TEMP_NOW, *(uint16_t *)&temp, TIME_OUT_MS);
    
    // rtu_write_reg(DEVICE_ID, SCREEN_TEMP_TREND, info->temp_trend, TIME_OUT_MS);

    temp = (int16_t )info->agrs->SetTempMax;
    rtu_write_reg(DEVICE_ID, SCREEN_TEMP_MAX, *(uint16_t *)&temp, TIME_OUT_MS);

    temp = (int16_t )info->agrs->SetTempMin;
    rtu_write_reg(DEVICE_ID, SCREEN_TEMP_MIN, *(uint16_t *)&temp, TIME_OUT_MS);
	/*方风机状态*/
	if(strcmp(info->device->MoSta, "forward") == 0)
	{
		rtu_write_reg(DEVICE_ID, SCREEN_MOTOR_STATUS, SCREEN_MOTOR_FORWARD, TIME_OUT_MS);
	}else if(strcmp(info->device->MoSta, "reverse") == 0)
	{
		rtu_write_reg(DEVICE_ID, SCREEN_MOTOR_STATUS, SCREEN_MOTOR_REVERSE, TIME_OUT_MS);
	}else if(strcmp(info->device->MoSta, "stop") == 0)
	{
		rtu_write_reg(DEVICE_ID, SCREEN_MOTOR_STATUS, SCREEN_MOTOR_STOP, TIME_OUT_MS);
	}else{
		MDF_LOGI("undefine stroke status");
		MDF_ERROR_ASSERT(-1);
	}
	/*方风机模式*/
    if(strcmp(info->device->ConSta, "manual") == 0)
	{
		rtu_write_reg(DEVICE_ID, SCREEN_CONTROL_MODE, SCREEN_CONTROL_MANUAL, TIME_OUT_MS);
	}else if(strcmp(info->device->ConSta, "auto") == 0)
	{
		rtu_write_reg(DEVICE_ID, SCREEN_CONTROL_MODE, SCREEN_CONTROL_AUTO, TIME_OUT_MS);
	}else
	{
		MDF_LOGI("undefine control mode");
        MDF_ERROR_ASSERT(-1);
	}
}

static void scan_task(void *argc)
{
    for(;;)
    {
        xSemaphoreTake(g_lock, SCAN_PEROID_MS / portTICK_PERIOD_MS);

        if(g_info[offset] == NULL)
            continue;
        screen_info_refresh(g_info[offset]);
    }
}
/*
 *屏幕初始化
*/
mdf_err_t screen_init(void)
{
	static bool init = false;
    if(init == false)
    {   
        g_lock = xSemaphoreCreateBinary();
        rtu_init();
        xTaskCreate(scan_task, "scan task", SCAN_TASK_SIZE, NULL, SCAN_TASK_PRI, NULL);
        init = true;
    }
	build_screen_info(&moter_flag1, &moter_args1,CONFIG_DEVICE_NUM * 2 - 1);
	build_screen_info(&moter_flag2, &moter_args2,CONFIG_DEVICE_NUM * 2);
	return MDF_OK;
}

void build_screen_info(moter_stu *device, moter_args *args,int id)
{
    screen_info_t *info = MDF_CALLOC(1, sizeof(screen_info_t));
    info->device = device;
	info->agrs = args;
    info->en = 1;
	info->id = id;
    for(int n = 0; n < MAX_INFO; n++)
    {
        if(g_info[n] == NULL)
        {
            g_info[n] = info;
            return;
        }
    }

    MDF_LOGI("screen info overflow");
    // MDF_ERROR_ASSERT(-1);
}

int get_drive_id()
{
	return g_info[offset]->id;
}

int change_screen_info()
{
    offset += 1;
    if(offset >= MAX_INFO)
        offset = 0;
    if(g_info[offset] == NULL)
        return -1;
	MDF_LOGI("change_screen_info %d",g_info[offset]->id);
    return g_info[offset]->id;
}

void set_screen_info(int id)
{
    offset = id + 1;
    if(offset >= MAX_INFO)
        offset = 0;
    if(g_info[offset] == NULL)
        return ;
	MDF_LOGI("set_screen_info %d",offset);
    return ;
}

void trig_screen_info_refresh()
{
    xSemaphoreGive(g_lock);
}
