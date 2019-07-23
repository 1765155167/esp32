#include "mdf-mesh.h"
#include "driver/uart.h"
#include "key.h"
#include "led.h"
static const char *TAG = "mdf-mesh";

uint8_t dest_addr[2][MWIFI_ADDR_LEN] = {
	{0x3c, 0x71, 0xbf, 0xe0, 0x92, 0xb8},
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x1c}
};

/**
 * @brief uart initialization
 */
static mdf_err_t uart_initialize()
{
    uart_config_t uart_config = {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    MDF_ERROR_ASSERT(uart_param_config(CONFIG_UART_PORT_NUM, &uart_config));
    MDF_ERROR_ASSERT(uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TX_IO, CONFIG_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    MDF_ERROR_ASSERT(uart_driver_install(CONFIG_UART_PORT_NUM, 2*BUF_SIZE, 2*BUF_SIZE, 0, NULL, 0));
    return MDF_OK;
}

/**
 * @接收串口发来的信息并通过mesh转发出去
 */
static void uart_handle_task(void *arg)
{
    int recv_length   = 0;
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
    uint8_t sta_mac[MWIFI_ADDR_LEN]   = {0};

    MDF_LOGI("Uart handle task is running");

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    /* uart initialization */
    MDF_ERROR_ASSERT(uart_initialize());
	uart_write_bytes(CONFIG_UART_PORT_NUM, "uart init ok\r\n", 14);//串口能收到

	
    while (1) {
        memset(data, 0, BUF_SIZE);
		recv_length = uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (recv_length <= 0) {
			MDF_LOGI("recv_length = %d",recv_length);
            continue;
        }
		data[recv_length] = 0;
        MDF_LOGI("UART Recv data:%s recv_length:%d", data, recv_length);
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char*)data, recv_length+1);//串口不能收到
        
		json_root = cJSON_Parse((char *)data);
        MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error, data: %s", data);
		
		json_dev = cJSON_GetObjectItem(json_root, "Devs");
		
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

			/**
			 * @brief  Convert mac from string format to binary
			 */
			do {
				//sscanf(json_id->valuestring, "%d", &id);
				id = json_id->valueint;
				printf("id: %d\n",id);
			} while (0);

			jsonstring = cJSON_PrintUnformatted(json_dev->child);

			if(id == 1) { /*数据发给自己的不需要转发*/
				//信息处理(自己)
				MDF_LOGI("自己的信息");
				json_led_press(jsonstring);
			}else if(id == -1)/*发给所有人*/
			{
				//信息处理(自己)
				MDF_LOGI("自己的信息");
				for(int i = 1; i < 2; i++)
				{
					ret = mwifi_write(dest_addr[i], &data_type, jsonstring, size, true);
					MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
				}
			} else
			{
				ret = mwifi_write(dest_addr[id-1], &data_type, jsonstring, size, true);
				MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
			}
			json_dev->child = json_dev->child->next;
		}
        
FREE_MEM:
        // MDF_FREE(recv_data);
        MDF_FREE(jsonstring);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("Uart handle task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 *@接收信息并打印
 **/
static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Note read task is running");

    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_read, ret: %x", ret);
        MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, \ndata: %s", MAC2STR(src_addr), size, data);
		json_led_press(data);
	}

    MDF_LOGW("Note read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}
/**
 *@ROOT接收信息通过串口发送
 **/
static void root_read_task(void *arg)
{
    mdf_err_t ret                    = MDF_OK;
    char *data                       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                      = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type      = {0};

	// uart_write_bytes(CONFIG_UART_PORT_NUM, "Root is running\n", 20);
    
	MDF_LOGI("Root is running");

    for (int i = 0;; ++i) {
        if (!mwifi_is_started()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_read", mdf_err_to_name(ret));
        MDF_LOGI("Root receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
		MDF_LOGI("uart send start");
        /* forwoad to uart */
        uart_write_bytes(CONFIG_UART_PORT_NUM, data, size);
        uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
		MDF_LOGI("uart sned ok");
	}

    MDF_LOGW("ROOT read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/*mesh write*/
mdf_err_t mesh_write(uint8_t src_addr[],char *data)
{
    mdf_err_t err = MDF_OK;
    size_t size   = strlen(data);
    mwifi_data_type_t data_type = {0x0};
    MDF_LOGI("Node write src_addr = %p size = %d data = %s",src_addr, size, data);

	if (!mwifi_is_connected()) {
		vTaskDelay(500 / portTICK_RATE_MS);
		return MDF_FAIL;
	}
	
	err = mwifi_write(src_addr, &data_type, data, size, true);
	MDF_ERROR_GOTO(err != MDF_OK, ret, "mwifi_write, err: %x", err);
	MDF_LOGI("mesh_write ok");
ret:
    return err;
}

/**
 * @brief Timed printing system information
 */
static void print_system_info_timercb(void *timer)
{
    uint8_t primary                 = 0;
    wifi_second_chan_t second       = 0;
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    mesh_assoc_t mesh_assoc         = {0x0};
    wifi_sta_list_t wifi_sta_list   = {0x0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_wifi_vnd_mesh_get(&mesh_assoc);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: " MACSTR ", parent bssid: " MACSTR
             ", parent rssi: %d, node num: %d, free heap: %u", primary,
             esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
             mesh_assoc.rssi, esp_mesh_get_total_node_num(), esp_get_free_heap_size());

    for (int i = 0; i < wifi_sta_list.num; i++) {
        MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
    }
	// mwifi_print_config();/* 打印　mesh　配置信息 */
#ifdef MEMORY_DEBUG
    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
#endif /**< MEMORY_DEBUG */
}

static mdf_err_t wifi_init()
{
    mdf_err_t ret          = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    tcpip_adapter_init();
    MDF_ERROR_ASSERT(esp_event_loop_init(NULL, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("Parent is connected on station interface");
            break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
            break;

        default:
            break;
    }

    return MDF_OK;
}

mdf_err_t mdf_mesh_init()
{
	mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config   = {
        .channel   = CONFIG_MESH_CHANNEL,
        .mesh_id   = CONFIG_MESH_ID,
        .mesh_type = CONFIG_DEVICE_TYPE,
    };
	MDF_LOGI("start mdf_mesh_init");
    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());
	
    /**
     * @brief Data transfer between wifi mesh devices
     */
    if (config.mesh_type == MESH_ROOT) {
		MDF_LOGI("MESH_ROOT");
        xTaskCreate(uart_handle_task, "uart_handle_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
		xTaskCreate(root_read_task, "root_read_task", 2 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    } else {
		MDF_LOGI("MESH_NODE");
        xTaskCreate(node_read_task, "node_read_task", 2 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }
	/* 定时打印 */
    TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, print_system_info_timercb);
    xTimerStart(timer, 0);
	return MDF_OK;
}

/*追加root标志*/
void data_add_root(char * data)
{
    char path[30] = ":{Root}";
    strncat(data, path, 1000);  // 1000远远超过path的长度
}

/*追加mac地址*/
void data_add_mac(char * data,uint8_t src_addr[])
{
	char * mac = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	sprintf(mac,":" MACSTR "",MAC2STR(src_addr));
    strncat(data, mac, 1000);  // 1000远远超过path的长度
	MDF_FREE(mac);
}