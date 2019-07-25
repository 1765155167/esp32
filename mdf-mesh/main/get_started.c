/*
 *
 */

#include "mdf_common.h"
#include "mwifi.h"
#include "mdf-mesh.h"
#include "key.h"
#include "led.h"

static const char *TAG = "get_started";

void app_main()
{
	MDF_ERROR_ASSERT( mdf_mesh_init() );
	MDF_ERROR_ASSERT( led_init() );
	MDF_ERROR_ASSERT( moter_init() );
	MDF_ERROR_ASSERT( key_init() );
	for(;;)
    {   
        vTaskDelay(3000/ portTICK_PERIOD_MS);//循环打印剩余内存
        MDF_LOGI("heap free: %d", esp_get_free_heap_size());
    }
}
