#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "driver/gpio.h"

static const char *TAG = "wifi";

//回调函数
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id)
	{
	case SYSTEM_EVENT_AP_START:/* 开始AP */
        printf("SYSTEM_EVENT_AP_START\n");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:/* 有STA链接ESP32的AP */
        printf("SYSTEM_EVENT_AP_STACONNECTED\n");
		ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d", 
					MAC2STR(event->event_info.sta_connected.mac),
                 	event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:/* 有STA断开ESP32链接 */
        printf("SYSTEM_EVENT_AP_STADISCONNECTED\n");
			ESP_LOGI(TAG, "station:"MACSTR" leave, AID=%d",
                	MAC2STR(event->event_info.sta_disconnected.mac),
                	event->event_info.sta_disconnected.aid);
        break;
    default:
        break;
	}
	return ESP_OK;
}

static void wifi_scan(void *arg)
{
	while (pdTRUE)
	{
		printf("...\n");
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	
}

void app_main() 
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	//初始化tcp/ip适配层
	tcpip_adapter_init();
	//注册回调函数
	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	//导入WIFI默认配置
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	//初始化wifi驱动
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

	wifi_config_t wifi_config = {
		.ap = {
			.ssid = "Mr.Hu",
			.ssid_len = 0,
			.max_connection = 1,
			.password = "hqf666123",
			.authmode = WIFI_AUTH_WPA2_PSK
		}
	};
	//设置wifi模式
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
	//配置wifi接口参数
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
	//wifi状态机开始运转
	ESP_ERROR_CHECK( esp_wifi_start() );

	xTaskCreate( wifi_scan, "wifi_scan", 4096, NULL, 10, NULL );
}