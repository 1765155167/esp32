#include "config.h"
#include "key.h"
#include "led.h"
#include "tcp.h"
#include "mwifi.h"
#include "mdf_common.h"
#include "mupgrade.h"
#include "mesh_mupgrade.h"

static const char *TAG = "get_started";

void app_main()
{
	mdf_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }
	MDF_ERROR_ASSERT(ret);
	
	MDF_ERROR_ASSERT( key_init() );
	MDF_ERROR_ASSERT( led_init() );
	MDF_ERROR_ASSERT( mupgrade_init() );
	
	for(;;)
	{
		vTaskDelay(3000/ portTICK_PERIOD_MS);//循环打印剩余内存
		MDF_LOGI("heap free: %d", esp_get_free_heap_size());
	}
}