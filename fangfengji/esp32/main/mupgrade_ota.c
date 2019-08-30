#include "mupgrade_ota.h"
#include "mdf-mesh.h"
#include "check.h"

static const char *TAG = "mupgrade_ota";
extern xQueueHandle g_queue_handle;
mdf_err_t get_version()/*获取版本信息*/
{
	size_t size;
	int i = 0;
	char * data = (char *) MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	MDF_LOGI("start 准备ota升级......");
	sprintf((char *)data,"{\"Typ\":\"cut_ota\",\"Seq\":0}");
	
	size = strlen((char *)data);
	uart_encryption((uint8_t *)data,&size,DUPLEX_NEED_ACK,STR);/*加密　crc检验位*/
	send_lock();
	uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);
	send_unlock();
	while(get_air202() != true) {
		i ++;
		send_lock();
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);
		send_unlock();
		if(i > 4)
		{
			MDF_LOGI("发送OTA升级信息失败");
		}
	}
	MDF_FREE(data);
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
    // vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);


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
	get_version();/*发送指令给Air202准备升级*/
	MDF_LOGI("创建OTA升级任务");
	xTaskCreate(mupgrade_ota, "mupgrade_ota", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);	
	
EXIT:
	cJSON_Delete(json_root);
	return ;
}
