#include "mupgrade.h"
#include "mdf_common.h"

static const char *TAG = "mesh_restart";

EventGroupHandle_t res_event_group;
static void restart_task(void * arg);
#define RES_EVENT BIT0

void mesh_restart(void)
{
    //发送led.c灯状态改变事件
    xEventGroupSetBits(res_event_group, RES_EVENT);
    MDF_LOGI("开始重启...");
}

mdf_err_t restart_init(void)
{
    static bool flag = false;
    if(flag) {
        return MDF_FAIL;
    }
    flag = true;
    //事件组
	res_event_group = xEventGroupCreate();
    xTaskCreate(restart_task, "led_control_task", 4 * 1024,
					NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    return MDF_OK;
}

static void restart_task(void * arg)
{
    EventBits_t uxBits;
	while(true)
	{
		//死等事件组：RES_EVENT
		uxBits = xEventGroupWaitBits(res_event_group, RES_EVENT, true, false, portMAX_DELAY);
		if (uxBits & RES_EVENT)
		{
			esp_restart();
		}
	}
}