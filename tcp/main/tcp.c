#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "driver/gpio.h"

//client
//STA模式配置信息,即要连上的路由器的账号密码
#define WIFI_SSID				"Ubuntu"        //账号
#define WIFI_PASS				"hqf666123"         //密码
#define TCP_SERVER_ADRESS       "10.42.0.1"         //作为client，要连接TCP服务器地址
#define TCP_PORT                1234                //统一的端口号，包括TCP客户端或者服务端

#define WIFI_CONNECTED_BIT BIT0
static const char *TAG = "tcp_client";

/* 全局变量定义 */
EventGroupHandle_t tcp_event_group;                     //wifi建立成功信号量
static struct sockaddr_in server_addr;                  //server地址
static int connect_socket = 0;                          //连接socket
bool g_rxtx_need_restart = false;                       //异常后，重新连接标记

/* 回调函数 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id)
	{
	case SYSTEM_EVENT_STA_START:        //STA模式-开始连接
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED: //STA模式-断线
        esp_wifi_connect();
        xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:    //STA模式-连接成功
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:       //STA模式-获取IP
        ESP_LOGI(TAG, "got ip:%s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
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
/* close_socket */
void close_socket()
{
    close(connect_socket);
    //close(server_socket);
}
/* 获取socket错误代码 */
int get_socket_error_code(int socket)
{
    int result;
    u32_t optlen = sizeof(int);
    int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen);
    if (err == -1)
    {
        //WSAGetLastError();
        ESP_LOGE(TAG, "socket error code:%d", err);
        ESP_LOGE(TAG, "socket error code:%s", strerror(err));
        return -1;
    }
    return result;
}
/* 获取socket错误原因 */
int show_socket_error_reason(const char *str, int socket)
{
    int err = get_socket_error_code(socket);

    if (err != 0)
    {
        ESP_LOGW(TAG, "%s socket error reason %d %s", str, err, strerror(err));
    }

    return err;
}
/* 接收数据 */
static void recv_data(void *arg)
{
    int len = 0;            //长度
    char databuff[64];      //缓存
    while (1)
    {
        //清空缓存
        memset(databuff, 0x00, sizeof(databuff));
        //读取接收数据
        len = recv(connect_socket, databuff, sizeof(databuff), 0);
        g_rxtx_need_restart = false;
        if (len > 0)
        {
            //g_total_data += len;
            //打印接收到的数组
            ESP_LOGI(TAG, "recvData: %s", databuff);
            //接收数据回发
            send(connect_socket, databuff, strlen(databuff), 0);
			if(strcmp("this is return...",databuff) == 0)
			{
				break;
			}
        }
        else
        {
            //打印错误信息
            show_socket_error_reason("recv_data", connect_socket);
            //服务器故障，标记重连
            g_rxtx_need_restart = true;
            break;
        }
    }
	printf("close socket\n");
    close_socket();
    //标记重连
    g_rxtx_need_restart = true;
    vTaskDelete(NULL);
}
/* 建立tcp client */
esp_err_t create_tcp_client()
{
    ESP_LOGI(TAG, "will connect gateway ssid : %s port:%d",
             TCP_SERVER_ADRESS, TCP_PORT);
    //新建socket
    connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_socket < 0)
    {
        //打印报错信息
        show_socket_error_reason("create client", connect_socket);
        //新建失败后，关闭新建的socket，等待下次新建
        close(connect_socket);
        return ESP_FAIL;
    }
    //配置连接服务器信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_ADRESS);
    ESP_LOGI(TAG, "connectting server...");
    //连接服务器
    if (connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        //打印报错信息
        show_socket_error_reason("client connect", connect_socket);
        ESP_LOGE(TAG, "connect failed!");
        //连接失败后，关闭之前新建的socket，等待下次新建
        close(connect_socket);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "connect success!");
    return ESP_OK;
}
/* 建立TCP连接并从TCP接收数据 */
static void tcp_connect(void *arg)
{
	while (1)
    {
        g_rxtx_need_restart = false;
        //等待WIFI连接信号量，死等
        xEventGroupWaitBits(tcp_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
        ESP_LOGI(TAG, "start tcp connected");
        TaskHandle_t tx_rx_task = NULL;
        //延时3S准备建立clien
        vTaskDelay(3000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "create tcp Client");
        //建立client
        int socket_ret = create_tcp_client();
        if (socket_ret == ESP_FAIL)//建立失败
        {
            ESP_LOGI(TAG, "create tcp socket error,stop...");
            continue;
        }
        else //建立成功
        {
            ESP_LOGI(TAG, "create tcp socket succeed...");            
            //建立tcp接收数据任务
            if (pdPASS != xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task))
            {
                //建立失败
                ESP_LOGI(TAG, "Recv task create fail!");
            }
            else
            {
                //建立成功
                ESP_LOGI(TAG, "Recv task create succeed!");
            }
        }
        while (1)
        {
            vTaskDelay(3000 / portTICK_RATE_MS);
            //重新建立client，流程和上面一样
            if (g_rxtx_need_restart) 
            {
                vTaskDelay(3000 / portTICK_RATE_MS);
                ESP_LOGI(TAG, "reStart create tcp client...");
                //建立client
                int socket_ret = create_tcp_client();

                if (socket_ret == ESP_FAIL) //建立失败
                {
                    ESP_LOGE(TAG, "reStart create tcp socket error,stop...");
                    continue;
                }
                else //建立成功
                {
                    ESP_LOGI(TAG, "reStart create tcp socket succeed...");
                    //重新建立完成，清除标记
                    g_rxtx_need_restart = false;
                    //建立tcp接收数据任务
                    if (pdPASS != xTaskCreate(&recv_data, "recv_data", 4096, NULL, 4, &tx_rx_task))
                    {
                        ESP_LOGE(TAG, "reStart Recv task create fail!");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "reStart Recv task create succeed!");
                    }
                }
            }
        }
    }

    vTaskDelete(NULL);
}

//配置wifi为STA模式
void wifi_init_sta()
{
	tcp_event_group = xEventGroupCreate();
	//初始化tcp/ip适配层
	tcpip_adapter_init();
	//注册回调函数
	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	//导入WIFI默认配置
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	//初始化wifi驱动
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = WIFI_SSID,
			.password = WIFI_PASS,
		}
	};
	//设置wifi模式
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	//配置wifi接口参数
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	//wifi状态机开始运转
	ESP_ERROR_CHECK( esp_wifi_start() );
	//打印系统日志
	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s\n",
	WIFI_SSID, WIFI_PASS);
}

void app_main() 
{
	//初始化flash
	printf("hello tcp_clent\n");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	//client，建立sta
    wifi_init_sta();
	xTaskCreate( tcp_connect, "tcp_connect", 4096, NULL, 10, NULL );
}