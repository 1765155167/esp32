#include "serial.h"
#include "mupgrade_ota.h"

#define ACK_EVENT BIT0
extern uint8_t dest_addr[3][MWIFI_ADDR_LEN];
static const char *TAG = "mesh-serial";
xQueueHandle g_queue_handle       = NULL;
static xSemaphoreHandle g_send_lock;
static xSemaphoreHandle _recv_lock;
static void uart_handle_task(void *arg);
static void serial_wait_ack(void *arg);
static EventGroupHandle_t uart_event_group;
static bool air202 = false;
void send_lock(void) {
    xSemaphoreTake(g_send_lock, portMAX_DELAY);
}
void send_unlock(void) {
    xSemaphoreGive(g_send_lock);
}
static void recv_lock_(void) {
    xSemaphoreTake(_recv_lock, portMAX_DELAY);
}
static void recv_unlock_(void) {
    xSemaphoreGive(_recv_lock);
}
/**
 * @brief uart initialization
 */
mdf_err_t uart_initialize(void)
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	init_flag = pdTRUE;
	//串口参数配置->uart1
	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(CONFIG_UART_PORT_NUM, &uart_config);		//设置串口
	//IO映射-> T:IO2  R:IO15
	uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TX_IO, CONFIG_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(CONFIG_UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
	
	g_send_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(g_send_lock);

	_recv_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(_recv_lock);

	uart_event_group = xEventGroupCreate();
	
	g_queue_handle = xQueueCreate(10, sizeof(queue_data_t));
    xTaskCreate(uart_handle_task, "uart_handle_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	xTaskCreate(serial_wait_ack, "serial_wait_ack", 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY - 2, NULL);
	return MDF_OK;
}

/**
 * @接收串口发来的信息并通过mesh转发出去
 */
static void uart_handle_task(void *arg)
{
	mdf_err_t err;
	uint8_t ack_typ;
	uint8_t typ;
    size_t recv_length   = 0;
	uint32_t id       = 0;
    mdf_err_t ret     = MDF_OK;
    cJSON *json_root  = NULL;
	cJSON *json_dev = NULL;
    cJSON *json_id    = NULL;

    // Configure a temporary buffer for the incoming data
    uint8_t *data                     = (uint8_t *) MDF_MALLOC(BUF_SIZE);
    size_t size                       = MWIFI_PAYLOAD_LEN;
    char *jsonstring                  = NULL;
    mwifi_data_type_t data_type       = {0};
	wifi_sta_list_t wifi_sta_list   = {0x0};

    MDF_LOGI("Uart handle task is running");
	
    while (1) {
        memset(data, 0, BUF_SIZE);
		recv_lock_();
		recv_length = uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
		recv_unlock_();
		if ((int)recv_length <= 0) {
			// MDF_LOGI("recv_length = %d",recv_length);
            continue;
		}

		err = uart_decrypt(data,(size_t * )&recv_length,&ack_typ,&typ);//串口数据解密
		MDF_ERROR_CONTINUE(err == MDF_FAIL,"uart recv data crc error!recv_length = %d",recv_length);
		
		if(ack_typ == DUPLEX_NEED_ACK) send_ack();/*发送应答信号*/
		else if(ack_typ == DUPLEX_IS_ACK){/*发送接收到ACK事件,暂时未用到*/
			xEventGroupSetBits(uart_event_group, ACK_EVENT);
			continue;
		} 

		if(typ == BIN)
		{
			queue_data_t q_data = {0x0};
            q_data.data = MDF_MALLOC(recv_length);
            q_data.size = recv_length;
			memcpy(q_data.data,data,recv_length);

			if (xQueueSend(g_queue_handle, &q_data, 0) != pdPASS) {
                MDF_LOGD("OTA Send receive queue failed");
                MDF_FREE(q_data.data);
            }
			// MDF_LOGI("data:BIN,size:%d",recv_length);
			continue;
		}
		// MDF_LOGI("data:STR,size:%d",recv_length);
		MDF_ERROR_CONTINUE(typ != STR,"uart recv data type is not STR!");
		
		MDF_LOGI("uart recv data %s,len = %d", data, recv_length);
		json_root = cJSON_Parse((char *)data);
        MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error, data: %s", data);
		
		json_dev = cJSON_GetObjectItem(json_root, "Devs");
		MDF_ERROR_CONTINUE(!json_dev, "json_root, data format error, data: %s", json_root->valuestring);

        while(json_dev->child)
		{
			json_id = cJSON_GetObjectItem(json_dev->child, "ID");

			if (json_id) {/* json id 存在 */
				data_type.group = false;
			} else {
				MDF_LOGW("ID not found");
				cJSON_Delete(json_root);
				continue;
			}

			id = json_id->valueint;
			MDF_LOGI("id: %d",id);

			jsonstring = cJSON_PrintUnformatted(json_dev->child);

			if(id == 1 || id == 2) { /*数据发给自己的不需要转发*/
				//信息处理(自己)
				MDF_LOGI("自己的信息");
				json_led_press(jsonstring);
			} else if(id == -1) {/*发给所有人*/
				//信息处理(自己)
				MDF_LOGI("发给所有人");
				json_led_press(jsonstring);
				esp_wifi_ap_get_sta_list(&wifi_sta_list);
				for (int i = 0; i < wifi_sta_list.num; i++) {
					MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
					ret = mwifi_write(wifi_sta_list.sta[i].mac, &data_type, jsonstring, size, true);
					MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
				}
			} else {
				ret = mwifi_write(dest_addr[(id-1)/2], &data_type, jsonstring, size, true);
				MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
			}
			json_dev->child = json_dev->child->next;
		}
        
FREE_MEM:
        MDF_FREE(jsonstring);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("Uart handle task is exit");
    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void serial_wait_ack(void *arg)
{
    EventBits_t uxBits;
	while (true)
	{
		uxBits = xEventGroupWaitBits(uart_event_group, ACK_EVENT, true, false, portMAX_DELAY);
		if (uxBits & ACK_EVENT)
		{
			air202  = true;
		}
	}
    vTaskDelete(NULL);
}

bool get_air202()
{
	EventBits_t uxBits;
	uxBits = xEventGroupWaitBits(uart_event_group, ACK_EVENT, true, false, 20 * 1000 / portTICK_PERIOD_MS);
	if (uxBits & ACK_EVENT)
	{
		return  true;
	}
	return false;
}