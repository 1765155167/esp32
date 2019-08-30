/*
 *@请参考README.md
 */
#include "key.h"
#include "led.h"
#include "mwifi.h"
#include "check.h"
#include "serial.h"
#include "mdf-mesh.h"
#include "temp_info.h"
#include "moter_nvs.h"
#include "mdf_common.h"
#include "screen_info.h"
#include "mupgrade_ota.h"

static const char *TAG = "get_started";
extern int CONFIG_DEVICE_NUM;/*设备号*/
extern int DEVICE_TYPE;/*设备类型*/

void app_main()
{
	MDF_ERROR_ASSERT( nvs_init() );
	MDF_ERROR_ASSERT( nvs_load() );
	MDF_ERROR_ASSERT( led_init() );
	MDF_ERROR_ASSERT( moter_init() );
	MDF_ERROR_ASSERT( key_init() );
	MDF_ERROR_ASSERT( screen_init() );/*屏幕*/
	MDF_ERROR_ASSERT( uart_initialize() );
	/*检查是否接收到Air202的信息*/
	if(get_air202() == true) {
		DEVICE_TYPE = MWIFI_MESH_ROOT;
		if(nvs_load_mac() != MDF_OK) {
			MDF_LOGI("请配置MAC地址表格\n:{ \"ID\": 1,\"Cmd\": \"mac\",\"Mac\":\"30:ae:a4:dd:b0:1c,3c:71:bf:e0:92:28,30:ae:a4:dd:b0:52\"}");
		}
	}else {
		DEVICE_TYPE = MWIFI_MESH_NODE;
	}
	MDF_ERROR_ASSERT( mdf_mesh_init() );
	MDF_LOGI("Hello 所有设备初始化已完成");
	
	vTaskDelay(10000 / portTICK_PERIOD_MS);

	for(;;)
	{
		vTaskDelay(3000 / portTICK_PERIOD_MS);//循环打印剩余内存
		MDF_LOGI("heap free: %d", esp_get_free_heap_size());
	}
}

