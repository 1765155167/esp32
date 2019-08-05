/*
 *
 */

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "temp_info.h"
#include "moter_nvs.h"
#include "key.h"
#include "led.h"


static const char *TAG = "get_started";

void app_main()
{
	MDF_ERROR_ASSERT( nvs_init() );
	MDF_ERROR_ASSERT( nvs_load() );
	MDF_ERROR_ASSERT( mdf_mesh_init() );
	MDF_ERROR_ASSERT( led_init() );
	MDF_ERROR_ASSERT( moter_init() );
	MDF_ERROR_ASSERT( key_init() );
	// MDF_ERROR_ASSERT( moter_temp_init() );
	for(;;)
	{
		vTaskDelay(3000/ portTICK_PERIOD_MS);//循环打印剩余内存
		MDF_LOGI("heap free: %d", esp_get_free_heap_size());
	}
}

mdf_err_t moter_restart()
{	
	/*保存信息*/
	nvs_save_OpenPer(1);
	nvs_save_arg(1);
	nvs_save_OpenPer(2);
	nvs_save_arg(2);
	esp_restart();//重启
}