/*
 *
 */

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "temp_info.h"
#include "moter_nvs.h"
#include "screen_info.h"
#include "mupgrade_ota.h"
#include "key.h"
#include "led.h"


static const char *TAG = "get_started";

void app_main()
{
	// uint8_t *data = (uint8_t *) MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	MDF_ERROR_ASSERT( nvs_init() );
	MDF_ERROR_ASSERT( nvs_load() );
	MDF_ERROR_ASSERT( mdf_mesh_init() );
	MDF_ERROR_ASSERT( led_init() );
	MDF_ERROR_ASSERT( moter_init() );
	MDF_ERROR_ASSERT( key_init() );
	MDF_ERROR_ASSERT( screen_init() );/*屏幕*/
	MDF_LOGI("Hello .................");
	for(;;)
	{
		vTaskDelay(3000/ portTICK_PERIOD_MS);//循环打印剩余内存
		MDF_LOGI("heap free: %d", esp_get_free_heap_size());
	}
}

