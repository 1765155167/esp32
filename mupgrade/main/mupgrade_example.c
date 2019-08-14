/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "mwifi.h"
#include "mupgrade.h"
#include "driver/uart.h"
#define BUF_SIZE 2048
//ack_typ
#define DUPLEX_NEED_ACK 2
#define DUPLEX_IS_ACK 3
#define DUPLEX_NO_ACK 1
//typ
#define STR 1
#define BIN 2
#define CONFIG_UART_TX_IO 21
#define CONFIG_UART_RX_IO 15
#define CONFIG_UART_PORT_NUM 2

unsigned char crc8_com(unsigned char *p, int counter);
mdf_err_t uart_decrypt( unsigned char * data,
						size_t * size,
						uint8_t * ack_typ,
						uint8_t * typ);//解密
void uart_encryption(	unsigned char * data,
						size_t * size,
						uint8_t ack_typ, 
						uint8_t typ);//加密
// mdf_err_t uart_decrypt(unsigned char * data, size_t *len);//解密
// void uart_encryption(unsigned char * data, size_t *len);//加密1
void send_ack(void);/*发送应答信号*/

static const char *TAG = "mupgrade_example";
// static xSemaphoreHandle g_send_lock;

void send_ack(void)/*发送应答信号*/
{
	uint8_t *data = (uint8_t *) MDF_MALLOC(BUF_SIZE);
	size_t size;
	uart_encryption(data, &size, DUPLEX_IS_ACK, STR);//加密
	uart_write_bytes(CONFIG_UART_PORT_NUM, (char*)data, size);
	uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
	MDF_FREE(data);
}

mdf_err_t uart_decrypt(unsigned char * data,
						 size_t * size,
						 uint8_t * ack_typ,
						 uint8_t * typ)//解密
{
	mdf_err_t err;
	size_t len = *size;
	uint8_t id;
	uint8_t crc;
	id = data[4];
	*ack_typ = data[5];
	*typ = data[6];

	MDF_LOGI("uart id:%d,ack_typ:%d,typ:%d",id,*ack_typ,*typ);
	
	crc = crc8_com(data + 4, len - 5);
	if(crc == data[len - 1]) {
		MDF_LOGI("crc check success crc:%d",crc);
		err = MDF_OK;
	}else {
		MDF_LOGI("crc check failure crc:%d,data_crc:%d",crc,data[len - 1]);
		err = MDF_FAIL;
	}
	data[len - 2] = '\0';
	for(int i = 0; i < len; i++)
	{
		data[i] = data[i + 7];
	}
	
	*size = len - 9;
	return err;
}

void uart_encryption(	unsigned char * data,
						size_t * size,
						uint8_t ack_typ, 
						uint8_t typ)//加密
{
	uint8_t crc;
	uint8_t id = 1;
	size_t len = *size;
	for(int i = len; i >= 0; i--)
	{
		data[i + 7] = data[i];
	}
	data[0] = '\0';
	data[1] = '\0';
	data[2] = (len + 3) / 256;
	data[3] = (len + 3) % 256;
	data[4] = id;
	data[5] = ack_typ;
	data[6] = typ;
	len = len + 7;
	crc = crc8_com(data + 4, len - 4);
	data[len] = crc;
	data[len + 1] = '\0';
	*size = len + 1;
	return ;
}

//CRC - 8 x8+x2+x+1
unsigned char crc8_com(unsigned char *p, int counter)
{
    unsigned char crc8 = 0;

    unsigned char crc_array[256] = {
        0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F,
        0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79,
        0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53,
        0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
        0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97,
        0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
        0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC,
        0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
        0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88,
        0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E,
        0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04,
        0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
        0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E,
        0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8,
        0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2,
        0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
        0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56,
        0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
        0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A,
        0x33, 0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
        0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39,
        0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F,
        0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5,
        0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
        0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1,
        0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3 
    };

    for (; counter > 0; counter--)
    {
        crc8 = crc_array[crc8^*p]; //查表得到CRC码
        p++;
    }
    return crc8;
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
	
	//串口配置结构体
	uart_config_t uart_config;
	//串口参数配置->uart1
	uart_config.baud_rate = 115200;					    //波特率
	uart_config.data_bits = UART_DATA_8_BITS;			//数据位
	uart_config.parity = UART_PARITY_DISABLE;			//校验位
	uart_config.stop_bits = UART_STOP_BITS_1;			//停止位
	uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	//硬件流控
	uart_param_config(CONFIG_UART_PORT_NUM, &uart_config);		//设置串口
	//IO映射-> T:IO21  R:IO15
	uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TX_IO, CONFIG_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	//注册串口服务即使能+设置缓存区大小
	uart_driver_install(CONFIG_UART_PORT_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
	
	// g_send_lock = xSemaphoreCreateBinary();
    // xSemaphoreGive(g_send_lock);
	init_flag = pdTRUE;
	return MDF_OK;
}

static void ota_task()
{
    mdf_err_t ret       = MDF_OK;
    uint8_t *data       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    char name[32]       = {0x0};
    size_t total_size   = 0;
    int start_time      = 0;
    mupgrade_result_t upgrade_result = {0};
    mwifi_data_type_t data_type = {.communicate = MWIFI_COMMUNICATE_MULTICAST};
	
    /**
     * @note If you need to upgrade all devices, pass MWIFI_ADDR_ANY;
     *       If you upgrade the incoming address list to the specified device
     */
    // uint8_t dest_addr[][MWIFI_ADDR_LEN] = {{0x1, 0x1, 0x1, 0x1, 0x1, 0x1}, {0x2, 0x2, 0x2, 0x2, 0x2, 0x2},};
    uint8_t dest_addr[][MWIFI_ADDR_LEN] = {MWIFI_ADDR_ANY};

    /**
     * @brief In order to allow more nodes to join the mesh network for firmware upgrade,
     *      in the example we will start the firmware upgrade after 30 seconds.
     */
    vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);

    esp_http_client_config_t config = {
        .url            = CONFIG_FIRMWARE_UPGRADE_URL,
        .transport_type = HTTP_TRANSPORT_UNKNOWN,
    };

    /**
     * @brief 1. Connect to the server
     */
    esp_http_client_handle_t client = esp_http_client_init(&config);
    MDF_ERROR_GOTO(!client, EXIT, "Initialise HTTP connection");

    start_time = xTaskGetTickCount();

    MDF_LOGI("Open HTTP connection: %s", CONFIG_FIRMWARE_UPGRADE_URL);

    /**
     * @brief First, the firmware is obtained from the http server and stored on the root node.
     */
    do {
        ret = esp_http_client_open(client, 0);

        if (ret != MDF_OK) {
            if (!esp_mesh_is_root()) {
                goto EXIT;
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
            MDF_LOGW("<%s> Connection service failed", mdf_err_to_name(ret));
        }
    } while (ret != MDF_OK);
/**
  * @brief这个函数需要在esp_http_client_open之后调用，它会从http流中读取，处理所有接收头
 *
  * @param [in] client esp_http_client句柄
 *
  * @return
  *  - （0）如果stream不包含content-length头或chunked编码（由`esp_http_client_is_chunked`响应检查）
  *  - （-1：ESP_FAIL）如果有任何错误
  *  - 下载内容长度标题定义的数据长度
 */
    total_size = esp_http_client_fetch_headers(client);
    sscanf(CONFIG_FIRMWARE_UPGRADE_URL, "%*[^//]//%*[^/]/%[^.bin]", name);

    if (total_size <= 0) {
        MDF_LOGW("Please check the address of the server");
        ret = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
        MDF_ERROR_GOTO(ret < 0, EXIT, "<%s> Read data from http stream", mdf_err_to_name(ret));

        MDF_LOGW("Recv data: %.*s", ret, data);
        goto EXIT;
    }

    /**
     * @brief 2. Initialize the upgrade status and erase the upgrade partition.
	 * @brief 2. 初始化升级状态并清除升级分区。
     */
    ret = mupgrade_firmware_init(name, total_size);
	sprintf((char *)data,"{\"Devs\":[{\"ID\": 1,\"Cmd\": \"OAT\",\"Params\": {\"table_size\": %d, \"name\": %s}}]}",total_size,name);
	size_t size1;
	MDF_LOGI("data %s",(char *)data);
	uart_encryption(data, &size1, DUPLEX_NEED_ACK, STR);//加密
	uart_write_bytes(CONFIG_UART_PORT_NUM, (char*)data, size1);
	uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);

	vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
	uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> Initialize the upgrade status", mdf_err_to_name(ret));

    /**
     * @brief 3. Read firmware from the server and write it to the flash of the root node
	 * @brief 3. 从服务器读取固件并将其写入根节点的闪存
     */
    for (ssize_t size = 0, recv_size = 0; recv_size < total_size; recv_size += size) {
        size = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
        MDF_ERROR_GOTO(size < 0, EXIT, "<%s> Read data from http stream", mdf_err_to_name(ret));
		uart_encryption(data, &size1, DUPLEX_NEED_ACK, STR);//加密
		uart_write_bytes(CONFIG_UART_PORT_NUM, (char*)data, size1);
		uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (size > 0) {
            /* @brief  Write firmware to flash */
            ret = mupgrade_firmware_download(data, size);
            MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> Write firmware to flash, size: %d, data: %.*s",
                           mdf_err_to_name(ret), size, size, data);
        } else {
            MDF_LOGW("<%s> esp_http_client_read", mdf_err_to_name(ret));
            goto EXIT;
        }
    }

    MDF_LOGI("The service download firmware is complete, Spend time: %ds",
             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);

    start_time = xTaskGetTickCount();

    /**
     * @brief 4. The firmware will be sent to each node.
	 * @brief 4. 固件将发送到每个节点。
     */
    ret = mupgrade_firmware_send((uint8_t *)dest_addr, sizeof(dest_addr) / MWIFI_ADDR_LEN, &upgrade_result);
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> mupgrade_firmware_send", mdf_err_to_name(ret));

    if (upgrade_result.successed_num == 0) {
        MDF_LOGW("Devices upgrade failed, unfinished_num: %d", upgrade_result.unfinished_num);
        goto EXIT;
    }

    MDF_LOGI("Firmware is sent to the device to complete, Spend time: %ds",
             (xTaskGetTickCount() - start_time) * portTICK_RATE_MS / 1000);
    MDF_LOGI("Devices upgrade completed, successed_num: %d, unfinished_num: %d", upgrade_result.successed_num, upgrade_result.unfinished_num);

    /**
     * @brief 5. the root notifies nodes to restart
	 * @brief 5. 根通知节点重新启动.
     */
    const char *restart_str = "restart";
    ret = mwifi_root_write(upgrade_result.successed_addr, upgrade_result.successed_num,
                           &data_type, restart_str, strlen(restart_str), true);
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

EXIT:
    MDF_FREE(data);
    mupgrade_result_free(&upgrade_result);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
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
    switch (event) {
        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("MDF_EVENT_PARENT_CONNECTED");
            // xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
            //             NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

            // if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
            //     xTaskCreate(root_read_task, "root_read_task", 4 * 1024,
            //                 NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            // }

            break;

        case MDF_EVENT_MWIFI_ROOT_GOT_IP:
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_GOT_IP");
            xTaskCreate(ota_task, "ota_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
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

void app_main()
{
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config   = {
        .router_ssid     = CONFIG_ROUTER_SSID,
        .router_password = CONFIG_ROUTER_PASSWORD,
        .mesh_id         = CONFIG_MESH_ID,
    };

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mupgrade_node", ESP_LOG_DEBUG);
    esp_log_level_set("mupgrade_root", ESP_LOG_DEBUG);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
	/**
	 * UART　init
	*/
	uart_initialize();
    MDF_LOGI("Starting OTA example ...");

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());
}
