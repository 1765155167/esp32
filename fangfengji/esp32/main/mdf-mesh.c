#include "mdf-mesh.h"
#include "driver/uart.h"
#include "key.h"
#include "led.h"
#include "check.h"
#include "mupgrade_ota.h"

static const char *TAG = "mdf-mesh";
int CONFIG_DEVICE_NUM = 1;/*设备号*/
int DEVICE_TYPE;/*设备类型*/
uint8_t dest_addr[3][MWIFI_ADDR_LEN] = {
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x1c},//root dev1
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x7c},//node dev2
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x68},//node dev3
};

/**
 *@重启设备
 */
static mdf_err_t moter_restart()
{	
	/*保存信息*/
	nvs_save_OpenPer(1);
	nvs_save_arg(1);
	nvs_save_OpenPer(2);
	nvs_save_arg(2);
	esp_restart();//重启
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
        if (!mwifi_is_connected()&& !(mwifi_is_started())) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
		MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));
		
		if (data_type.upgrade) { // This mesh package contains upgrade data.
			MDF_LOGI("This mesh package contains upgrade data.node");
            ret = mupgrade_handle(src_addr, data, size);/*处理ROOT发送的升级数据*/
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_handle", mdf_err_to_name(ret));
        } else {
			MDF_LOGI("Receive [ROOT] addr: " MACSTR ", size: %d, data: %s",
                     MAC2STR(src_addr), size, data);

			/**
             * @brief Finally, the node receives a restart notification. Restart it yourself..
             */
            if (!strcmp(data, "restart")) {
                MDF_LOGI("Restart the version of the switching device");
                MDF_LOGW("The device will restart after 3 seconds");
                vTaskDelay(pdMS_TO_TICKS(3000));
                // esp_restart();
				moter_restart();
            }
			json_led_press(data);
		}
	}

    MDF_LOGW("Note read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}
/*
 *@字符串拷贝函数
 * */
static void mystrcpy(char *des,char *src)
{
    while(*src != '\0')
	{
		*des++ = *src++;
	}
	*des = '\0';
}
/*处理从设备发送的信息*/
static mdf_err_t root_recv_data_process(char * data)
{
	mdf_err_t err = MDF_OK;
	cJSON * json_id;
	cJSON * json_cmd;
	cJSON  * json_root;
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	MDF_ERROR_GOTO(!json_id, ret, "json_id, data format error, data: %s", data);

	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	MDF_ERROR_GOTO(!json_cmd, ret, "json_cmd, data format error, data: %s", data);
	
	if(strcmp(json_cmd->valuestring,"tempAlarm") == 0)
	{
		/*温度报警*/
		add_dev_info(data);/*追加信息*/
		size_t size = strlen(data);
		MDF_LOGI("温度报警:%s",data);
		
		send_lock();
		uart_encryption((uint8_t *)data,&size,DUPLEX_NO_ACK,STR);/*加密　crc检验位*/
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);
        uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
		send_unlock();
	}else/*从设备上传的状态信息*/
	{
		switch (json_id->valueint)
		{
		case 3:
			moter_3 = pdTRUE;
			mystrcpy(json_moter_3,data);
			break;
		case 4:
			moter_4 = pdTRUE;
			mystrcpy(json_moter_4,data);
			break;
		case 5:
			moter_5 = pdTRUE;
			mystrcpy(json_moter_5,data);
			break;
		case 6:
			moter_6 = pdTRUE;
			mystrcpy(json_moter_6,data);
			break;
		default:
			break;
		}
	}
	
ret:
	cJSON_Delete(json_root);	
	return err;
}
/**
 *@ROOT接收从设备发送的信息
 **/
static void root_read_task(void *arg)
{
    mdf_err_t ret                    = MDF_OK;
    char * data                      = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                      = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type      = {0};

	MDF_LOGI("Root read is running");

    for (int i = 0;; ++i) {
        if (!mwifi_is_connected() && !(mwifi_is_started())) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
		MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));
		
		if (data_type.upgrade) { // This mesh package contains upgrade data.
			MDF_LOGI("This mesh package contains upgrade data.root");
            ret = mupgrade_root_handle(src_addr, data, size);/*ota*/
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_root_handle", mdf_err_to_name(ret));
        } else {
			MDF_LOGI("Receive [NODE] addr: " MACSTR ", size: %d, data: %s",
                     MAC2STR(src_addr), size, data);
			
			root_recv_data_process(data);/*处理从设备发送的信息*/
		}
	}

    MDF_LOGW("ROOT read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @mesh write
 */
mdf_err_t mesh_write(uint8_t src_addr[],char *data)
{
    mdf_err_t err = MDF_OK;
    size_t size   = strlen(data);
    mwifi_data_type_t data_type = {0x0};
    // MDF_LOGI("Node write src_addr = %p size = %d data = %s",src_addr, size, data);

	if (!mwifi_is_connected() && !(mwifi_is_started())) {
		vTaskDelay(500 / portTICK_RATE_MS);
		MDF_LOGW("mwifi is not connected.");
		return MDF_FAIL;
	}
	
	err = mwifi_write(src_addr, &data_type, data, size, true);
	MDF_ERROR_GOTO(err != MDF_OK, EXIT, "mwifi_write, err: %x", err);
	MDF_LOGI("mesh_write ok");
EXIT:
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
        MDF_ERROR_ASSERT( nvs_flash_erase() );
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    tcpip_adapter_init();
    MDF_ERROR_ASSERT( esp_event_loop_init(NULL, NULL) );
    MDF_ERROR_ASSERT( esp_wifi_init(&cfg) );
    MDF_ERROR_ASSERT( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
    MDF_ERROR_ASSERT( esp_wifi_set_mode(WIFI_MODE_STA) );
    MDF_ERROR_ASSERT( esp_wifi_set_ps(WIFI_PS_NONE) );
    MDF_ERROR_ASSERT( esp_mesh_set_6m_rate(false) );
    MDF_ERROR_ASSERT( esp_wifi_start() );

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
            if(esp_mesh_get_layer() == MESH_ROOT_LAYER)
            {
                led_status_unset(MESH_CONNECTION_ERROR);
            }
            MDF_LOGI("MESH is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
			led_status_unset(MESH_CONNECTION_ERROR);
            MDF_LOGI("Parent is connected on station interface");
            
            break;
        case MDF_EVENT_MWIFI_CHILD_CONNECTED:
            if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
                mesh_write(dest_addr[0],"{\"ID\":1,\"Cmd\":\"set mac\"}");
                mesh_write(dest_addr[1],"{\"ID\":3,\"Cmd\":\"set mac\"}");
                mesh_write(dest_addr[2],"{\"ID\":5,\"Cmd\":\"set mac\"}");
            }
            led_status_unset(MESH_CONNECTION_ERROR);
            MDF_LOGI("MDF_EVENT_MWIFI_CHILD_CONNECTED");
            break;
        case MDF_EVENT_MWIFI_CHILD_DISCONNECTED:
            led_status_set(MESH_CONNECTION_ERROR);
            MDF_LOGI("MDF_EVENT_MWIFI_CHILD_DISCONNECTED");
            break;
        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
			led_status_set(MESH_CONNECTION_ERROR);
            break;
		
		case MDF_EVENT_MUPGRADE_STARTED: {
            mupgrade_status_t status = {0x0};
            mupgrade_get_status(&status);

            MDF_LOGI("MDF_EVENT_MUPGRADE_STARTED, name: %s, size: %d",
                     status.name, status.total_size);
            break;
        }

        case MDF_EVENT_MUPGRADE_STATUS:
            MDF_LOGI("Upgrade progress: %d%%", (int)ctx);
            break;

        default:
            break;
    }

    return MDF_OK;
}

mdf_err_t mdf_mesh_init()
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	init_flag = pdTRUE;
	MDF_LOGI("start mdf_mesh_init");
    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
	mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));

    mwifi_config_t config   = {
        .channel   = CONFIG_MESH_CHANNEL,
        .mesh_id   = CONFIG_MESH_ID,
        .mesh_type = DEVICE_TYPE,
    };

    MDF_ERROR_ASSERT( mwifi_set_config(&config) );
    MDF_ERROR_ASSERT( mwifi_start() );
    /**
     * @brief Data transfer between wifi mesh devices
     */
    if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
		MDF_LOGI("MESH_ROOT,layre:%d",esp_mesh_get_layer());
		xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);	
		xTaskCreate(root_read_task, "root_read_task", 4 * 1024,
					NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    } else {
		MDF_LOGI("MESH_NODE,layer:%d",esp_mesh_get_layer());
		xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }
	/* 定时打印 10s*/
    TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, print_system_info_timercb);
    xTimerStart(timer, 0);
	
	return MDF_OK;
}
