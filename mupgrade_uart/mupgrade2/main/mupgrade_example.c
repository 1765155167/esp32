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
#include "check.h"

#define BUF_SIZE MWIFI_PAYLOAD_LEN
static const char *TAG = "mupgrade_example";
static xSemaphoreHandle g_send_lock;
static xSemaphoreHandle _recv_lock;
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
	uart_param_config(1, &uart_config);		//设置串口
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


static void root_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Root read task is running");

    while (mwifi_is_connected()) {
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

        if (data_type.upgrade) { // This mesh package contains upgrade data.
            MDF_LOGI("This mesh package contains upgrade data. root");
            ret = mupgrade_root_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_root_handle", mdf_err_to_name(ret));
        } else {
            MDF_LOGI("Receive [NODE] addr: " MACSTR ", size: %d, data: %s",
                     MAC2STR(src_addr), size, data);
        }
    }

    MDF_LOGW("Root read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @brief Handling data between wifi mesh devices.
 */
static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Node read task is running");

    while (mwifi_is_connected()) {
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

        if (data_type.upgrade) { // This mesh package contains upgrade data.
            MDF_LOGI("This mesh package contains upgrade data. node");
            ret = mupgrade_handle(src_addr, data, size);
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
                esp_restart();
            }
        }
    }

    MDF_LOGW("Node read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void ota_task()
{
    mdf_err_t ret       = MDF_OK;
    uint8_t *data       = MDF_MALLOC(MWIFI_PAYLOAD_LEN + 8);
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
     */
    ret = mupgrade_firmware_init(name, total_size);
    MDF_ERROR_GOTO(ret != MDF_OK, EXIT, "<%s> Initialize the upgrade status", mdf_err_to_name(ret));

    /**
     * @brief 3. Read firmware from the server and write it to the flash of the root node
     */
    for (ssize_t size = 0, recv_size = 0; recv_size < total_size; recv_size += size) {
        size = esp_http_client_read(client, (char *)data, 1024);
        MDF_LOGI("%02x%02x %02x%02x,size:%d",data[0],data[1],data[size - 2],data[size - 1],size);
        //uart_encryption((uint8_t *)data,(size_t *)&size,DUPLEX_NO_ACK,BIN);/*加密　crc检验位*/
        //MDF_LOGI("size:%d",size);
        send_lock();
        uart_write_bytes(CONFIG_UART_PORT_NUM, (char *)data, size);
        send_unlock();
        MDF_ERROR_GOTO(size < 0, EXIT, "<%s> Read data from http stream", mdf_err_to_name(ret));

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
    vTaskDelay(100 * 1000 / portTICK_PERIOD_MS);
    /**
     * @brief 4. The firmware will be sent to each node.
     */
    MDF_LOGI("4. The firmware will be sent to each node.");
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
     */
    MDF_LOGI("5. the root notifies nodes to restart");
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
            xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

            if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
                xTaskCreate(root_read_task, "root_read_task", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            }

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