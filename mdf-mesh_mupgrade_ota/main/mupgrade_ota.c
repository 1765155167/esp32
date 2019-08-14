#include "mupgrade_ota.h"
#include "mdf-mesh.h"
#include "check.h"
static const char *TAG = "mupgrade_ota";

size_t total_size  = 0;
BaseType_t  get_ota_flag = pdFALSE;

mdf_err_t get_version(unsigned char * data)/*获取版本信息*/
{
	size_t size;
	// mdf_err_t err;
	// uint8_t ack_typ;
	// uint8_t typ;
	// char *data1 = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
	// sprintf(data1, "{\"ID\": 1,\"Cmd\": \"ota\",\"Params\": {\"table_size\": 869952, \"name\": \"mdf-mesh.bin\"}}");
	MDF_LOGI("start get_version 获取版本信息");
	sprintf((char *)data,"{\"Typ\":\"cut_ota\",\"Seq\":0}");
	//DUPLEX_NO_ACK//str
	send_lock();
	size = strlen((char *)data);
	uart_encryption((uint8_t *)data,&size,DUPLEX_NO_ACK,STR);/*加密　crc检验位*/
	uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);

	// mupgrade_ota(data1);	

	// MDF_FREE(data1);
	return MDF_OK;
}

int start_time     = 0;
void mupgrade_ota(char * data)
{
	mdf_err_t err;
	// uint8_t ack_typ;
	// uint8_t typ;
	
	cJSON *json_root   = NULL;
    cJSON *json_id     = NULL;
	cJSON *json_cmd    = NULL;
	cJSON *json_params = NULL;
	cJSON *json_size   = NULL;
	cJSON *json_name   = NULL;
	

	MDF_LOGI("start mupgrade_ota");
	json_root = cJSON_Parse((char *)data);
	MDF_ERROR_GOTO(!json_root, ret, "cJSON_Parse, data format error, data: %s", data);

	json_id = cJSON_GetObjectItem(json_root, "ID");
	if (!json_id) {/* json id 不存在 */
		MDF_LOGW("ID not found");
		goto ret;
	}
	json_cmd = cJSON_GetObjectItem(json_root, "Cmd");
	if (!json_cmd) {/* json cmd 不存在 */
		MDF_LOGW("Cmd not found");
		goto ret;
	}
	if(strcmp(json_cmd->valuestring,"ota") != 0) {
		MDF_LOGW("Cmd not is ota");
		goto ret;
	}
	json_params = cJSON_GetObjectItem(json_root, "Params");
	if (!json_params) {
		MDF_LOGW("Params not found");
		goto ret;
	}
	json_size = cJSON_GetObjectItem(json_params, "table_size");
	if (!json_size) {
		MDF_LOGW("table_size not found");
		goto ret;
	}
	json_name = cJSON_GetObjectItem(json_params, "name");
	if (!json_size) {
		MDF_LOGW("name not found");
		goto ret;
	}
	total_size = json_size->valueint;
	// vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
	start_time = xTaskGetTickCount();
	if (total_size <= 0) {
        MDF_LOGW("Please check the address of the server");
        goto ret;
    }
	/**
     * @brief 2. Initialize the upgrade status and erase the upgrade partition.
	 * @brief 2. 初始化升级状态并清除升级分区。
     */
	MDF_LOGI("2. 初始化升级状态并清除升级分区。");
    err = mupgrade_firmware_init(json_name->valuestring, total_size);
    MDF_ERROR_GOTO(err != MDF_OK, ret, "<%s> Initialize the upgrade status", mdf_err_to_name(err));
	/**
     * @brief 3. Read firmware from the server and write it to the flash of the root node
	 * @brief 3. 从服务器读取固件并将其写入根节点的闪存
     */
	MDF_LOGI("3. 从服务器读取固件并将其写入根节点的闪存");
	get_ota_flag = pdTRUE;
ret:
	cJSON_Delete(json_root);
    
	return ;
}

mdf_err_t set_ota_data(uint8_t * ota_data,size_t size)
{
	mdf_err_t err;
	mupgrade_result_t upgrade_result = {0};
	mwifi_data_type_t data_type = {.communicate = MWIFI_COMMUNICATE_MULTICAST};
	/**
     * @note If you need to upgrade all devices, pass MWIFI_ADDR_ANY;
     *       If you upgrade the incoming address list to the specified device
     */
    // uint8_t dest_addr[][MWIFI_ADDR_LEN] = {{0x1, 0x1, 0x1, 0x1, 0x1, 0x1}, {0x2, 0x2, 0x2, 0x2, 0x2, 0x2},};
    uint8_t dest_addr[][MWIFI_ADDR_LEN] = {MWIFI_ADDR_ANY};

	/* @brief  Write firmware to flash */
	if(!get_ota_flag)goto ret;
	err = mupgrade_firmware_download(ota_data, size);
	MDF_ERROR_GOTO(err != MDF_OK, ret, "<%s> Write firmware to flash, size: %d, ota_data: %.*s",
					mdf_err_to_name(err), size, size, ota_data);
	total_size -= size;
	MDF_LOGI("...................total_size = %d,size = %d",total_size,size);
	if(total_size <= 0)
	{
		// mupgrade_firmware_check(const esp_partition_t *partition);
		MDF_LOGI("The service download firmware is complete, Spend time: %ds", 
					(xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);

		start_time = xTaskGetTickCount();
		/**
		 * @brief 4. The firmware will be sent to each node.
		 * @brief 4. 固件将发送到每个节点。
		 */
		MDF_LOGI("4. 固件将发送到每个节点。");
		err = mupgrade_firmware_send((uint8_t *)dest_addr, sizeof(dest_addr) / MWIFI_ADDR_LEN, &upgrade_result);
		MDF_ERROR_GOTO(err != MDF_OK, ret, "<%s> mupgrade_firmware_send", mdf_err_to_name(err));
		
		if (upgrade_result.successed_num == 0) {
			MDF_LOGW("Devices upgrade failed, unfinished_num: %d", upgrade_result.unfinished_num);
			goto ret;
		}

		MDF_LOGI("Firmware is sent to the device to complete, Spend time: %ds",
				(xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);
		MDF_LOGI("Devices upgrade completed, successed_num: %d, unfinished_num: %d",
				upgrade_result.successed_num, upgrade_result.unfinished_num);

		/**
		 * @brief 5. the root notifies nodes to restart
		 * @brief 5. 根通知节点重新启动.
		 */
		const char *restart_str = "restart";
		err = mwifi_root_write(upgrade_result.successed_addr, upgrade_result.successed_num,
							&data_type, restart_str, strlen(restart_str), true);
		MDF_ERROR_GOTO(err != MDF_OK, ret, "<%s> mwifi_root_recv", mdf_err_to_name(err));
	}
	mupgrade_result_free(&upgrade_result);
	return MDF_OK;
ret:
	mupgrade_result_free(&upgrade_result);
	return MDF_FAIL;
}