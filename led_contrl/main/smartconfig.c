
/* Esptouch example
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

char WIFI_SSID[32] = {'\0'};
char WIFI_PASSWD[64] = {'\0'};

//wifi链接成功事件
static EventGroupHandle_t wifi_event_group;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "sc";
static const char *TAG1 = "u_event";


void smartconfig_example_task(void *parm);
void http_get_task(void *pvParameters);

/*
* wifi事件
* @param[in]   event  		       :事件
* @retval      esp_err_t           :错误类型
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/10, 初始化版本\n 
*/
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG1, "SYSTEM_EVENT_STA_START");
        //创建smartconfig任务
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG1, "SYSTEM_EVENT_STA_GOT_IP");
        //sta链接成功，set事件组
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG1, "SYSTEM_EVENT_STA_DISCONNECTED");
        //断线重连
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/*smartconfig事件回调
* @param[in]   status  		       :事件状态
* @retval      void                 :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/10, 初始化版本\n 
*/
wifi_config_t *wifi_config;
static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status)
    {
    case SC_STATUS_WAIT:                    //等待配网
        ESP_LOGI(TAG, "SC_STATUS_WAIT");
        break;
    case SC_STATUS_FIND_CHANNEL:            //扫描信道
        ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
        break;
    case SC_STATUS_GETTING_SSID_PSWD:       //获取到ssid和密码
        ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
        break;
    case SC_STATUS_LINK:                    //连接获取的ssid和密码
        ESP_LOGI(TAG, "SC_STATUS_LINK");
        wifi_config = pdata;
        //打印账号密码
        ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
        //断开默认的
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        //设置获取的ap和密码到寄存器
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config));
        //连接获取的ssid和密码
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SC_STATUS_LINK_OVER: //连接上配置后，结束
        ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
        //
        if (pdata != NULL)
        {
            uint8_t phone_ip[4] = {0};
            memcpy(phone_ip, (uint8_t *)pdata, 4);
            ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
        }
        //发送sc结束事件
        xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
        break;
    default:
        break;
    }
}
/*smartconfig任务
* @param[in]   void  		       :wu
* @retval      void                 :无
* @note        修改日志 
*               Ver0.0.1:
                    hx-zsj, 2018/08/10, 初始化版本\n 
*/
void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;
    //使用ESP-TOUCH配置
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    //开始sc
    ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));
    while (1)
    {
        //死等事件组：CONNECTED_BIT | ESPTOUCH_DONE_BIT
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        
        //sc结束
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
			esp_err_t err;
			nvs_handle wifi_handle;
            err = nvs_open("WIFI", NVS_READWRITE, &wifi_handle);
			if (err != ESP_OK) {
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			} else {
				err = nvs_set_str(wifi_handle, "ssid", (char *)wifi_config->sta.ssid);
				err = nvs_set_str(wifi_handle, "passwd", (char *)wifi_config->sta.password);
				err = nvs_commit(wifi_handle);
				printf((err != ESP_OK) ? "WIFI SSID PASSWD SAVE ERR\n" : "WIFI SSID PASSWD SAVE OK\n");
				nvs_close(wifi_handle);// Close
			}
			nvs_close(wifi_handle);// Close
			esp_restart();
            // xTaskCreate(http_get_task, "http_get_task", 4096, NULL, 3, NULL);
            vTaskDelete(NULL);
        }
        //连上ap
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
    }
}


static void initialise_wifi(void)
{
	ESP_LOGI(TAG,"START initialise_wifi...");
    // ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    //事件组
    wifi_event_group = xEventGroupCreate();
    //注册wifi事件
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    //wifi设置:默认设置，等待sc配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    //sta模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //启动wifi
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t get_wifi_info()
{
	esp_err_t err;
	size_t size_ssid = 32;
	size_t size_passwd = 64;
	nvs_handle wifi_handle;
	ESP_LOGI(TAG,"START GET WIFI INFO...");
	err = nvs_open("WIFI", NVS_READWRITE, &wifi_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		ESP_LOGI(TAG,"TRY LOAD WIFI INFO");
		err = nvs_get_str(wifi_handle, "ssid", WIFI_SSID, &size_ssid);
		if(err != ESP_OK) goto NVSFAL;
		
		err = nvs_get_str(wifi_handle, "passwd", WIFI_PASSWD, &size_passwd);
		if(err != ESP_OK) goto NVSFAL;
		
		err = nvs_commit(wifi_handle);
		if(err != ESP_OK)
		{
NVSFAL:
			ESP_LOGI(TAG,"WIFI SSID PASSWD LOAD ERR");
			initialise_wifi();
		}else
		{
			ESP_LOGI(TAG,"WIFI SSID PASSWD LOAD OK");
		}
	}
	nvs_close(wifi_handle);// Close
	return err;
}
