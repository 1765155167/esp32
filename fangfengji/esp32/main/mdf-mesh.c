#include "mdf-mesh.h"
#include "driver/uart.h"
#include "key.h"
#include "led.h"
#include "check.h"
#include "mupgrade_ota.h"

static const char *TAG = "mdf-mesh";
static xQueueHandle g_queue_handle       = NULL;
static xSemaphoreHandle g_send_lock;
static xSemaphoreHandle _recv_lock;
static void mupgrade_ota(void * arg);
int CONFIG_DEVICE_NUM = 1;/*设备号*/
int DEVICE_TYPE = MWIFI_MESH_ROOT;/*设备类型*/

typedef struct {/*OAT升级固件信息*/
    size_t size;     /**< Received size */
    uint8_t *data; /**< Received data */
} queue_data_t;

uint8_t dest_addr[3][MWIFI_ADDR_LEN] = {
	// {0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x1c},//root 1
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x7c},
	{0x30, 0xae, 0xa4, 0xdd, 0xb0, 0x68},
	{0x24, 0x6f, 0x28, 0xd9, 0x5f, 0x84},//node 1
	// {0x3c, 0x71, 0xbf, 0xe0, 0x92, 0xb8},//node 2
};
/* MAC地址比较 */
BaseType_t mac_cmp(uint8_t * sta_mac, uint8_t * dest_addr)
{
	int resault = pdTRUE;
	for(int i = 0; i < MWIFI_ADDR_LEN; i++)
	{
		if(sta_mac[i] != dest_addr[i])
		{
			resault = pdFALSE;
			break;
		}
	}
	return resault;
}

/*获取该设备的设备号*/
int get_dev_num()
{
	int num;
	uint8_t sta_mac[MWIFI_ADDR_LEN]   = {0};
	
    MDF_LOGI("get_dev_num");
    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

	for(num = 0;num < 3; num ++)/*总共3个设备*/
	{
		if(mac_cmp(sta_mac,dest_addr[num]))
		break;
	}
	if(num == 3) MDF_LOGE("该设备MAC地址不在dest_addr中");
	return num + 1;
}

void send_lock(void)
{
    xSemaphoreTake(g_send_lock, portMAX_DELAY);
}

void send_unlock(void)
{
    xSemaphoreGive(g_send_lock);
}
static void recv_lock_(void)
{
    xSemaphoreTake(_recv_lock, portMAX_DELAY);
}

static void recv_unlock_(void)
{
    xSemaphoreGive(_recv_lock);
}

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
 * @brief uart initialization
 */
static mdf_err_t uart_initialize(void)
{
	static bool init_flag = pdFALSE;
	if(init_flag) {
		return MDF_OK;
	}
	
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

	init_flag = pdTRUE;
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
    // uint8_t sta_mac[MWIFI_ADDR_LEN]   = {0};

    MDF_LOGI("Uart handle task is running");

    // esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    /* uart initialization */
    MDF_ERROR_ASSERT(uart_initialize());
	
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
			MDF_LOGI("data:BIN");
			continue;
		}
		MDF_LOGI("data:STR");
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
			printf("id: %d\n",id);

			jsonstring = cJSON_PrintUnformatted(json_dev->child);

			if(id == 1 || id == 2) { /*数据发给自己的不需要转发*/
				//信息处理(自己)
				MDF_LOGI("自己的信息");
				json_led_press(jsonstring);
			} else if(id == -1)/*发给所有人*/
			{
				//信息处理(自己)
				MDF_LOGI("自己的信息");
				json_led_press(jsonstring);
				esp_wifi_ap_get_sta_list(&wifi_sta_list);
				for (int i = 0; i < wifi_sta_list.num; i++) {
					MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
					ret = mwifi_write(wifi_sta_list.sta[i].mac, &data_type, jsonstring, size, true);
					MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
				}
			} else
			{
				ret = mwifi_write(dest_addr[(id-1)/2], &data_type, jsonstring, size, true);
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

	if (!mwifi_is_connected()) {
		vTaskDelay(500 / portTICK_RATE_MS);
		MDF_LOGW("mwifi is not connected.");
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

size_t total_size  = 0;
static void mupgrade_ota(void * arg)
{
	mdf_err_t err;
	int start_time      = 0;
    mupgrade_result_t upgrade_result = {0};
    mwifi_data_type_t data_type = {.communicate = MWIFI_COMMUNICATE_MULTICAST};	

	/**
     * @note If you need to upgrade all devices, pass MWIFI_ADDR_ANY;
     *       If you upgrade the incoming address list to the specified device
     */
    // uint8_t dest_addr[][MWIFI_ADDR_LEN] = {{0x1, 0x1, 0x1, 0x1, 0x1, 0x1}, {0x2, 0x2, 0x2, 0x2, 0x2, 0x2}};
    uint8_t dest_addr[][MWIFI_ADDR_LEN] = {MWIFI_ADDR_ANY};
	
	/**
     * @brief In order to allow more nodes to join the mesh network for firmware upgrade,
     *      in the example we will start the firmware upgrade after 30 seconds.
     */
    vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);


	start_time = xTaskGetTickCount();
	/**
     * @brief 2. Initialize the upgrade status and erase the upgrade partition.
	 * @brief 2. 初始化升级状态并清除升级分区。
     */
    err = mupgrade_firmware_init("hello-world.bin", total_size);
    MDF_ERROR_GOTO(err != MDF_OK, EXIT, "<%s> Initialize the upgrade status", mdf_err_to_name(err));
	
	/**
     * @brief 3. Read firmware from the server and write it to the flash of the root node
	 * @brief 3. 从服务器读取固件并将其写入根节点的闪存
     */
	MDF_LOGI("3. 从服务器读取固件并将其写入根节点的闪存");

	for (ssize_t size = 0, recv_size = 0; recv_size < total_size; recv_size += size) {
        // size = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
        
		queue_data_t q_data = {0x0};
		if (xQueueReceive(g_queue_handle, &q_data, 100000) != pdPASS) {
			MDF_LOGD("Read queue timeout");
			goto EXIT;
		}
		size = q_data.size;
		MDF_LOGI("%02x%02x %02x%02x",q_data.data[0],q_data.data[1],q_data.data[size - 2],q_data.data[size - 1]);
		MDF_ERROR_GOTO(size < 0, EXIT, "<%s> Read data from http stream", mdf_err_to_name(err));

        if (size > 0) {
            /* @brief  Write firmware to flash */
            err = mupgrade_firmware_download(q_data.data, size);
			MDF_FREE(q_data.data);
            MDF_ERROR_GOTO(err != MDF_OK, EXIT, "<%s> Write firmware to flash, size: %d, data: %.*s",
                           mdf_err_to_name(err), size, size, q_data.data);
        } else {
			MDF_FREE(q_data.data);
            MDF_LOGW("<%s> esp_http_client_read", mdf_err_to_name(err));
            goto EXIT;
        }
    }

    MDF_LOGI("The service download firmware is complete, Spend time: %ds",
             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);

    start_time = xTaskGetTickCount();

    /**
     * @brief 4. The firmware will be sent to each node.
     */
	MDF_LOGI("4. The firmware will be sent to each node.");
    err = mupgrade_firmware_send((uint8_t *)dest_addr, sizeof(dest_addr) / MWIFI_ADDR_LEN, &upgrade_result);
    MDF_ERROR_GOTO(err != MDF_OK, EXIT, "<%s> mupgrade_firmware_send", mdf_err_to_name(err));

    if (upgrade_result.successed_num == 0) {
        MDF_LOGW("Devices upgrade failed, unfinished_num: %d", upgrade_result.unfinished_num);
        goto EXIT;
    }

    MDF_LOGI("Firmware is sent to the device to complete, Spend time: %ds",
             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);
    MDF_LOGI("Devices upgrade completed, successed_num: %d, unfinished_num: %d", upgrade_result.successed_num, upgrade_result.unfinished_num);

    /**
     * @brief 5. the root notifies nodes to restart
     */
	MDF_LOGI("5. the root notifies nodes to restart");
    const char *restart_str = "restart";
    err = mwifi_root_write(upgrade_result.successed_addr, upgrade_result.successed_num,
                           &data_type, restart_str, strlen(restart_str), true);
    MDF_ERROR_GOTO(err != MDF_OK, EXIT, "<%s> mwifi_root_recv", mdf_err_to_name(err));

EXIT:
    mupgrade_result_free(&upgrade_result);
    // esp_http_client_close(client);
    // esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void esp_start_ota(char * data)
{
	mdf_err_t err = MDF_OK;
	cJSON *json_root   = NULL;
    cJSON *json_id     = NULL;
	cJSON *json_cmd    = NULL;
	cJSON *json_params = NULL;
	cJSON *json_size   = NULL;
	cJSON *json_name   = NULL;
	
	// char name[32]       = {0x0};
	MDF_LOGI("start mupgrade_ota");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, EXIT, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	MDF_ERROR_GOTO(!json_id, EXIT, "ID not found");

	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	MDF_ERROR_GOTO(!json_cmd, EXIT, "Cmd not found");
	
	if(strcmp(json_cmd->valuestring,"ota") != 0) {
		MDF_LOGW("Cmd not is ota");
		goto EXIT;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	MDF_ERROR_GOTO(!json_params, EXIT, "Params not found");

	json_size = cJSON_GetObjectItem(json_params, "table_size");
	MDF_ERROR_GOTO(!json_size, EXIT, "table_size not found");
	
	json_name = cJSON_GetObjectItem(json_params, "name");
	MDF_ERROR_GOTO(!json_name, EXIT, "name not found");
	
	// memcpy(name, json_name->valuestring, sizeof(json_name->valuestring));
	total_size = json_size->valueint;
	MDF_ERROR_GOTO((long int)total_size <= 0, EXIT, "total_size小于0");
	
	MDF_LOGI("创建OTA升级任务");
	MDF_ERROR_ASSERT( xTaskCreate(mupgrade_ota, "mupgrade_ota", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL) );	
EXIT:
	cJSON_Delete(json_root);
	return ;
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
			led_status_unset(MESH_CONNECTION_ERROR);
            MDF_LOGI("Parent is connected on station interface");
            
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

	CONFIG_DEVICE_NUM =  get_dev_num();/*获取该设备的设备号*/
	if(CONFIG_DEVICE_NUM == 1)
	{
		DEVICE_TYPE = MWIFI_MESH_ROOT;
		MDF_LOGI("ROOT device:%d",CONFIG_DEVICE_NUM);
	} else {
		DEVICE_TYPE = MWIFI_MESH_NODE;
		MDF_LOGI("NODE device:%d",CONFIG_DEVICE_NUM);
	}
	
    mwifi_config_t config   = {
        .channel   = CONFIG_MESH_CHANNEL,
        .mesh_id   = CONFIG_MESH_ID,
        .mesh_type = DEVICE_TYPE,
    };

	g_queue_handle = xQueueCreate(10, sizeof(queue_data_t));

    MDF_ERROR_ASSERT( mwifi_set_config(&config) );
    MDF_ERROR_ASSERT( mwifi_start() );
	led_status_set( MESH_CONNECTION_ERROR );
    /**
     * @brief Data transfer between wifi mesh devices
     */
    if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
		MDF_LOGI("MESH_ROOT,layre:%d",esp_mesh_get_layer());
        xTaskCreate(uart_handle_task, "uart_handle_task", 2 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
		xTaskCreate(node_read_task, "node_read_task", 2 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);	
		xTaskCreate(root_read_task, "root_read_task", 2 * 1024,
					NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    } else {
		MDF_LOGI("MESH_NODE,layer:%d",esp_mesh_get_layer());
		xTaskCreate(node_read_task, "node_read_task", 2 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }
	/* 定时打印 10s*/
    TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, print_system_info_timercb);
    xTimerStart(timer, 0);
	init_flag = pdTRUE;
	return MDF_OK;
}
