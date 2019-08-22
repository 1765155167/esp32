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

#include "key.h"
#include "led.h"
#include "tcp.h"
#include "mwifi.h"
#include "config.h"
#include "mupgrade.h"
#include "mdf_common.h"
#include "smartconfig.h"
#include "mesh_mupgrade.h"
#include "cmdProcessing.h"
#include "mesh_mqtt_handle.h"


static const char *TAG = "mupgrade_example";
extern char WIFI_SSID[32];
extern char WIFI_PASSWD[64];

typedef struct {
    size_t last_num;
    uint8_t *last_list;/*实际路由表信息*/
    size_t change_num;
    uint8_t *change_list;/*存放添加或者丢失的路由表信息*/
} node_list_t;

/*从addrs_list路由表中去除掉一条addr*/
static bool addrs_remove(uint8_t *addrs_list, size_t *addrs_num, const uint8_t *addr)
{
    for (int i = 0; i < *addrs_num; i++, addrs_list += MWIFI_ADDR_LEN) {
        if (!memcmp(addrs_list, addr, MWIFI_ADDR_LEN)) {
            if (--(*addrs_num)) {
                memcpy(addrs_list, addrs_list + MWIFI_ADDR_LEN, (*addrs_num - i) * MWIFI_ADDR_LEN);
            }
            return true;
        }
    }
    return false;
}

/*根节点接收mqtt信息*/
static void mqtt_read_task(void *arg)
{
	mdf_err_t ret = MDF_OK;
    char *data    = NULL;
    size_t size   = MWIFI_PAYLOAD_LEN;
    uint8_t dest_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type       = {0x0};

    MDF_LOGI("MQTT read task is running");

    while (mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT) {
        if (!mesh_mqtt_is_connect()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        /**
         * @brief Recv data from mqtt data queue, and forward to special device.
         */
        ret = mesh_mqtt_read(dest_addr, (void **)&data, &size, 500 / portTICK_PERIOD_MS);
		if(ret != MDF_OK) {
			goto MEM_FREE;
		}

        ret = mwifi_root_write(dest_addr, 1, &data_type, data, size, true);
        MDF_ERROR_GOTO(ret != MDF_OK, MEM_FREE, "<%s> mwifi_root_write", mdf_err_to_name(ret));

MEM_FREE:
        MDF_FREE(data);
    }

    MDF_LOGW("Root read task is exit");
    mesh_mqtt_stop();
    vTaskDelete(NULL);
}

/*接收发送到根节点的信息，在通过mqtt转发出去*/
static void root_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Root read task is running");

    while (true) {
		if(!mwifi_is_connected() && esp_mesh_get_layer() == MESH_ROOT && !mesh_mqtt_is_connect())
		{
			vTaskDelay(pdMS_TO_TICKS(300));
			continue;
		}
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

        if (data_type.upgrade) { // This mesh package contains upgrade data.
			MDF_LOGI(" This mesh package contains upgrade data. ROOT");
            ret = mupgrade_root_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_root_handle", mdf_err_to_name(ret));
        } else {
			MDF_LOGI("[ROOT] Receive addr: " MACSTR ", size: %d, data: %s",
                     MAC2STR(src_addr), size, data);
			ret = mesh_mqtt_write(src_addr, data, size);
			MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mesh_mqtt_publish", mdf_err_to_name(ret));
        }
    }

    MDF_LOGW("Root read task is exit");
	mesh_mqtt_stop();
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

    while (true) {
		if(!mwifi_is_connected())
		{
			vTaskDelay(pdMS_TO_TICKS(300));
			continue;
		}
		size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_recv", mdf_err_to_name(ret));

        if (data_type.upgrade) { // This mesh package contains upgrade data.
			MDF_LOGI(" This mesh package contains upgrade data. NODE");
            ret = mupgrade_handle(src_addr, data, size);
            MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mupgrade_handle", mdf_err_to_name(ret));
        } else {
            MDF_LOGI("[NODE] Receive addr: " MACSTR ", size: %d, data: %s",
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

			cmdProcessing(data);/*处理MQTT下发的指令*/
        }
    }

    MDF_LOGW("Node read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/*定时上传mac地址信息与所在层级*/
static void node_write_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    int count     = 0;
    size_t size   = 0;
    char *data    = NULL;
    mwifi_data_type_t data_type     = {0x0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Node task is running");

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    for (;;) {
        if (!mwifi_is_connected() || !mwifi_get_root_status()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        /**
         * @brief Send device information to mqtt server throught root node.
         */
        size = asprintf(&data, "{\"mac\": \"%02x%02x%02x%02x%02x%02x\", \"seq\":%d,\"layer\":%d}",
                        MAC2STR(sta_mac), count++, esp_mesh_get_layer());

        MDF_LOGD("Node send, size: %d, data: %s", size, data);
        ret = mwifi_write(NULL, &data_type, data, size, true);/*发送到根节点*/
        MDF_FREE(data);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_write", mdf_err_to_name(ret));

        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node task is exit");
    vTaskDelete(NULL);
}

/*ota升级任务*/
void ota_task(void * arg)
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
    vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);

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
        size = esp_http_client_read(client, (char *)data, MWIFI_PAYLOAD_LEN);
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

    /**
     * @brief 4. The firmware will be sent to each node.
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

// /*读取tcp服务器发送来的信息*/
// static void tcp_read_task(void *arg)
// {
// 	mdf_err_t err;
// 	size_t size = MWIFI_PAYLOAD_LEN;
// 	char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
// 	while(true)
// 	{
// 		vTaskDelay(20 * 1000 / portTICK_PERIOD_MS);
// 		size = MWIFI_PAYLOAD_LEN;
// 		err = tcp_client_read(data, &size);
// 		MDF_ERROR_CONTINUE(err != MDF_OK, "tcp client read err");
// 		MDF_LOGI("tcp read data:%s,size:%d",data,size);
// 		tcp_client_write(data);
// 		if(strcmp(data,"ota") == 0)
// 		{
// 			xTaskCreate(ota_task, "ota_task", 4 * 1024,
// 							NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
// 			goto RET;
// 		}
// 	}
// RET:
// 	MDF_FREE(data);
// 	vTaskDelete(NULL);
// }

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
	static node_list_t node_list = {0x0};

    switch (event) {
        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("MDF_EVENT_PARENT_CONNECTED");
            xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
			xTaskCreate(node_write_task, "node_write_task", 4 * 1024,
                        NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

            if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
                xTaskCreate(root_read_task, "root_read_task", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
            }

            break;

		case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");

            if (esp_mesh_is_root()) {
                mesh_mqtt_stop();
            }

            break;
		
		case MDF_EVENT_MWIFI_ROUTING_TABLE_ADD:
            MDF_LOGI("MDF_EVENT_MWIFI_ROUTING_TABLE_ADD, total_num: %d", esp_mesh_get_total_node_num());

            if (esp_mesh_is_root()) {

                /**
                 * @brief find new add device.
                 */
                node_list.change_num  = esp_mesh_get_routing_table_size();
                node_list.change_list = MDF_MALLOC(node_list.change_num * sizeof(mesh_addr_t));
                ESP_ERROR_CHECK(esp_mesh_get_routing_table((mesh_addr_t *)node_list.change_list,
                                node_list.change_num * sizeof(mesh_addr_t), (int *)&node_list.change_num));

                for (int i = 0; i < node_list.last_num; ++i) {
                    addrs_remove(node_list.change_list, &node_list.change_num, node_list.last_list + i * MWIFI_ADDR_LEN);
                }

                node_list.last_list = MDF_REALLOC(node_list.last_list,
                                                  (node_list.change_num + node_list.last_num) * MWIFI_ADDR_LEN);
                memcpy(node_list.last_list + node_list.last_num * MWIFI_ADDR_LEN,
                       node_list.change_list, node_list.change_num * MWIFI_ADDR_LEN);
                node_list.last_num += node_list.change_num;

                /**
                 * @brief subscribe topic for new node
                 */
                MDF_LOGI("total_num: %d, add_num: %d", node_list.last_num, node_list.change_num);
                mesh_mqtt_subscribe(node_list.change_list, node_list.change_num);
                MDF_FREE(node_list.change_list);
            }

            break;
		
		case MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE:
            MDF_LOGI("MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE, total_num: %d", esp_mesh_get_total_node_num());

            if (esp_mesh_is_root()) {
                /**
                 * @brief find removed device.
                 */
                size_t table_size      = esp_mesh_get_routing_table_size();
                uint8_t *routing_table = MDF_MALLOC(table_size * sizeof(mesh_addr_t));
                ESP_ERROR_CHECK(esp_mesh_get_routing_table((mesh_addr_t *)routing_table,
                                table_size * sizeof(mesh_addr_t), (int *)&table_size));

                for (int i = 0; i < table_size; ++i) {
                    addrs_remove(node_list.last_list, &node_list.last_num, routing_table + i * MWIFI_ADDR_LEN);
                }

                node_list.change_num  = node_list.last_num;
                node_list.change_list = MDF_MALLOC(node_list.last_num * MWIFI_ADDR_LEN);
                memcpy(node_list.change_list, node_list.last_list, node_list.change_num * MWIFI_ADDR_LEN);

                node_list.last_num  = table_size;
                memcpy(node_list.last_list, routing_table, table_size * MWIFI_ADDR_LEN);
                MDF_FREE(routing_table);

                /**
                 * @brief unsubscribe topic for leaved node
                 */
                MDF_LOGI("total_num: %d, add_num: %d", node_list.last_num, node_list.change_num);
                mesh_mqtt_unsubscribe(node_list.change_list, node_list.change_num);
                MDF_FREE(node_list.change_list);
            }

            break;

        case MDF_EVENT_MWIFI_ROOT_GOT_IP:
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_GOT_IP");

			mesh_mqtt_start(CONFIG_MQTT_URL);

			/**
             * @brief subscribe topic for all subnode
             */
            size_t table_size      = esp_mesh_get_routing_table_size();
            uint8_t *routing_table = MDF_MALLOC(table_size * sizeof(mesh_addr_t));
            ESP_ERROR_CHECK(esp_mesh_get_routing_table((mesh_addr_t *)routing_table,
                            table_size * sizeof(mesh_addr_t), (int *)&table_size));

            node_list.last_num  = table_size;
            node_list.last_list = MDF_REALLOC(node_list.last_list,
                                              node_list.last_num * MWIFI_ADDR_LEN);
            memcpy(node_list.last_list, routing_table, table_size * MWIFI_ADDR_LEN);
            MDF_FREE(routing_table);

            MDF_LOGI("subscribe %d node", node_list.last_num);
            mesh_mqtt_subscribe(node_list.last_list, node_list.last_num);
            MDF_FREE(node_list.change_list);

			if (esp_mesh_get_layer() == MESH_ROOT_LAYER) {
				xTaskCreate(mqtt_read_task, "mqtt_read_task", 4 * 1024,
							NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
			}
			// xTaskCreate(tcp_read_task, "tcp_read_task", 4 * 1024,
            //             NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

            break;

		case MDF_EVENT_CUSTOM_MQTT_CONNECT:
            MDF_LOGI("MQTT connect");
            mwifi_post_root_status(true);
            break;

        case MDF_EVENT_CUSTOM_MQTT_DISCONNECT:
            MDF_LOGI("MQTT disconnected");
            mwifi_post_root_status(false);
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
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
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

mdf_err_t mupgrade_init()
{
	MDF_LOGI("MUPGRATE INIT...");
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config   = {
        .mesh_id         = CONFIG_MESH_ID,
		.mesh_password   = CONFIG_MESH_PASSWORD,
    };
	if(strlen(WIFI_SSID) == 0)
	{
		MDF_LOGW("SSID:%s",config.router_ssid);
		return MDF_FAIL;
	}
	strcpy(config.router_ssid, WIFI_SSID);
	strcpy(config.router_password, WIFI_PASSWD);
	MDF_LOGI("SSID:%s",config.router_ssid);
	MDF_LOGI("PASSWD:%s",config.router_password);
    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mupgrade_node", ESP_LOG_DEBUG);
    esp_log_level_set("mupgrade_root", ESP_LOG_DEBUG);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    MDF_LOGI("Starting OTA example ...");

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());
	return MDF_OK;
}
