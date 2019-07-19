/* Mesh Manual Networking Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_mesh.h"
#include "esp_mesh_internal.h"
#include "nvs_flash.h"

/*******************************************************
 *                Macros
 *******************************************************/
//#define MESH_SET_ROOT

#ifndef MESH_SET_ROOT
#define MESH_SET_NODE
#endif

/*******************************************************
 *                Constants
 *******************************************************/
#define SEND_LEN 32
/*******************************************************
 *                Variable Definitions
 *******************************************************/
static const char *MESH_TAG = "mesh_main";
static const uint8_t MESH_ID[6]   = {0x75, 0x77, 0x77, 0x77, 0x77, 0x77};
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;/* 层 */
static bool is_mesh_connected = false;/* 是否链接进mesh网络的标志 */
static bool is_mesh_start = false;/* 是否链接进mesh网络的标志 */
/*******************************************************
 *                Function Declarations
 *******************************************************/
void mesh_event_handler(mesh_event_t event)
{
    mesh_addr_t id = {0,};
    static uint8_t last_layer = 0;
    ESP_LOGD(MESH_TAG, "esp_event_handler:%d", event.id);

    switch (event.id) {
    case MESH_EVENT_STARTED:/* mesh start */
        esp_mesh_get_id(&id);
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STARTED>ID:"MACSTR"", MAC2STR(id.addr));
        is_mesh_connected = false;
		is_mesh_start = true;
        mesh_layer = esp_mesh_get_layer();
        break;
    case MESH_EVENT_STOPPED:/* mesh stop */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
        is_mesh_connected = false;
		is_mesh_start = false;
        mesh_layer = esp_mesh_get_layer();
        break;
    case MESH_EVENT_CHILD_CONNECTED:/* < a child is connected on softAP interface > */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, "MACSTR"",
                 event.info.child_connected.aid,
                 MAC2STR(event.info.child_connected.mac));
        break;
    case MESH_EVENT_CHILD_DISCONNECTED:/* < a child is disconnected on softAP interface > */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, "MACSTR"",
                 event.info.child_disconnected.aid,
                 MAC2STR(event.info.child_disconnected.mac));
        break;
    case MESH_EVENT_ROUTING_TABLE_ADD:/* 添加一个新的节点从而改变路由表 */
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d",
                 event.info.routing_table.rt_size_change,
                 event.info.routing_table.rt_size_new);
        break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE:/* 移除一个新的节点从而改变路由表 */
        ESP_LOGW(MESH_TAG, "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d",
                 event.info.routing_table.rt_size_change,
                 event.info.routing_table.rt_size_new);
        break;
    case MESH_EVENT_NO_PARENT_FOUND:/* 没有发现父节点 */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
                 event.info.no_parent.scan_times);
        /* TODO handler for the failure */
        break;
    case MESH_EVENT_PARENT_CONNECTED:/* 连接到父节点 */
        esp_mesh_get_id(&id);
        mesh_layer = event.info.connected.self_layer;/* 层 */
        memcpy(&mesh_parent_addr.addr, event.info.connected.connected.bssid, 6);
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:"MACSTR"%s, ID:"MACSTR"",
                 last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "", MAC2STR(id.addr));
        last_layer = mesh_layer;
        is_mesh_connected = true;
        if (esp_mesh_is_root()) {/* 是根节点 */
            tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
		}
        break;
    case MESH_EVENT_PARENT_DISCONNECTED:/* 与父节点断开链接 */
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
                 event.info.disconnected.reason);
        is_mesh_connected = false;
        mesh_layer = esp_mesh_get_layer();
        break;
    case MESH_EVENT_LAYER_CHANGE:/* 设备层级发生改变 */
        mesh_layer = event.info.layer_change.new_layer;
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s",
                 last_layer, mesh_layer,
                 esp_mesh_is_root() ? "<ROOT>" :
                 (mesh_layer == 2) ? "<layer2>" : "");
        last_layer = mesh_layer;
        break;
    case MESH_EVENT_ROOT_ADDRESS:/* 获得根节点地址 */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:"MACSTR"",
                 MAC2STR(event.info.root_addr.addr));
        break;
    case MESH_EVENT_ROOT_GOT_IP:/* 根节点获取ip */
        /* root starts to connect to server */
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_GOT_IP>sta ip: " IPSTR ", mask: " IPSTR ", gw: " IPSTR,
                 IP2STR(&event.info.got_ip.ip_info.ip),
                 IP2STR(&event.info.got_ip.ip_info.netmask),
                 IP2STR(&event.info.got_ip.ip_info.gw));
        break;
    case MESH_EVENT_ROOT_LOST_IP:/* 根节点丢失ip地址 */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_LOST_IP>");
        break;
    case MESH_EVENT_TODS_STATE:/* 根是否能够访问外部网络的状态 */
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d",
                 event.info.toDS_state);
        break;
    case MESH_EVENT_ROOT_FIXED:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
                 event.info.root_fixed.is_fixed ? "fixed" : "not fixed");
        break;
    case MESH_EVENT_ROOT_ASKED_YIELD:
        ESP_LOGI(MESH_TAG,
                 "<MESH_EVENT_ROOT_ASKED_YIELD>"MACSTR", rssi:%d, capacity:%d",
                 MAC2STR(event.info.root_conflict.addr),
                 event.info.root_conflict.rssi,
                 event.info.root_conflict.capacity);
        break;
    case MESH_EVENT_CHANNEL_SWITCH:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d", event.info.channel_switch.channel);
        break;
    case MESH_EVENT_SCAN_DONE:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d",
                 event.info.scan_done.number);
        break;
    case MESH_EVENT_NETWORK_STATE:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
                 event.info.network_state.is_rootless);
        break;
    case MESH_EVENT_STOP_RECONNECTION:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
        break;
    case MESH_EVENT_FIND_NETWORK:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:"MACSTR"",
                 event.info.find_network.channel, MAC2STR(event.info.find_network.router_bssid));
        break;
    case MESH_EVENT_ROUTER_SWITCH:
        ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, "MACSTR"",
                 event.info.router_switch.ssid, event.info.router_switch.channel, MAC2STR(event.info.router_switch.bssid));
        break;
    default:
        ESP_LOGI(MESH_TAG, "unknown id:%d", event.id);
        break;
    }
}
#ifdef MESH_SET_ROOT
void root_task(void * arg)
{
	uint8_t addr[6];
	esp_err_t ret;
	mesh_addr_t route_table[5];
	int route_table_size = 0;
	// static int flag = MESH_DATA_P2P;
	// int timeout_ms = portMAX_DELAY;
	// static mesh_opt_t opt;
	char *c_data = (char *)malloc(SEND_LEN*sizeof(char));
	sprintf(c_data,"hello,I'am root\n");
	static mesh_data_t mesh_data;
	mesh_data.data = (uint8_t *)c_data;
	mesh_data.size = SEND_LEN;
	mesh_data.proto = MESH_PROTO_BIN;
	
	for(;;)
	{
		printf("root start...\n");
		printf("设备总数:%d\n", esp_mesh_get_total_node_num());
		esp_mesh_get_routing_table((mesh_addr_t *) &route_table, 5 * 6, &route_table_size);
		for (int i = 0; i < route_table_size; i++) {
			ret = esp_mesh_send(&route_table[i], &mesh_data, MESH_DATA_P2P, NULL, 0);
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
#else
void leaf_send_task(void * arg)
{
	static int flag = MESH_DATA_P2P;
	static mesh_opt_t opt;
	//opt.type = MESH_OPT_RECV_DS_ADDR;    /**< option type */
    //opt.len = SEND_LEN;    /**< option length */
    //opt.val = ;    /**< option value */
	char *c_data = (char *)malloc(SEND_LEN*sizeof(char));
	sprintf(c_data,"hello,I'am left\n");
	static mesh_data_t mesh_data;
	mesh_data.data = (uint8_t *)c_data;
	mesh_data.size = SEND_LEN;
	mesh_data.proto = MESH_PROTO_BIN;
	for(;;)
	{
		if(is_mesh_connected)
		{
			/* 打印发送的信息 */
			for(int i = 0;i < mesh_data.size; i++)
			{
				ESP_LOGI(MESH_TAG,"%c",mesh_data.data[i]);
			}
			//发送到根节点
			esp_mesh_send(NULL, &mesh_data, flag, NULL, 0);
			ESP_LOGI(MESH_TAG,"send ok!!!!!\n");
			ESP_LOGI(MESH_TAG,"设备总数:%d\n", esp_mesh_get_total_node_num());
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}


void leaf_recv_task(void * arg)
{
	mesh_addr_t from;
	esp_err_t ret;
	static uint8_t rx_buf[SEND_LEN] = { 0, };
	static int flag = MESH_DATA_P2P;
	int timeout_ms = portMAX_DELAY;
	static mesh_opt_t opt;
	static mesh_data_t mesh_data;
	mesh_data.data = rx_buf;
    mesh_data.size = SEND_LEN;
	for(;;)
	{
		if(is_mesh_connected)
		{
			printf("leaf_recv_task...\n");
			ret = esp_mesh_recv(&from, &mesh_data, timeout_ms, &flag, NULL, 0);
			if(ret == ESP_OK)
			{
				ESP_LOGI(MESH_TAG,"left read data....\n");
				/* 打印接收的信息 */ 
				for(int i = 0;i < mesh_data.size; i++)
				{
					printf("%c",mesh_data.data[i]);
				}
				printf("\nrecv_addr:%02x:%02x:%02x:%02x:%02x:%02x\n",MAC2STR(from.addr));
			}
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
#endif

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    /*  tcpip initialization */
    tcpip_adapter_init();
    /* for mesh
     * stop DHCP server on softAP interface by default
     * stop DHCP client on station interface by default
     * */
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA));
#if 1/*　设置静态ip 不使用路由器时必须手动设置 */
    /* static ip settings */
    tcpip_adapter_ip_info_t sta_ip;
#ifdef MESH_SET_ROOT
    sta_ip.ip.addr = ipaddr_addr("192.168.1.101");
#else
	sta_ip.ip.addr = ipaddr_addr("192.168.1.102");
#endif
    sta_ip.gw.addr = ipaddr_addr("192.168.1.1");
    sta_ip.netmask.addr = ipaddr_addr("255.255.255.0");
    tcpip_adapter_set_ip_info(WIFI_IF_STA, &sta_ip);
#endif
    /*  wifi initialization */
    ESP_ERROR_CHECK(esp_event_loop_init(NULL, NULL));
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_start());
    /*  mesh initialization */
    ESP_ERROR_CHECK(esp_mesh_init());
    /* mesh enable IE crypto */
    mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
    /* mesh ID */
    memcpy((uint8_t *) &cfg.mesh_id, MESH_ID, 6);
    /* mesh event callback */
    cfg.event_cb = &mesh_event_handler;
    cfg.channel = CONFIG_MESH_CHANNEL;
	esp_mesh_fix_root(true);

	/* 设置类型 MESH_ROOT根节点　MESH_LEAF叶子节点 */
#ifdef MESH_SET_ROOT
    esp_mesh_set_type(MESH_ROOT);
	ESP_LOGI(MESH_TAG,"MESH_ROOT");
	xTaskCreate(root_task, "root_task", 4096, NULL, 10, NULL);
#else
	//esp_mesh_set_type(MESH_LEAF);
	ESP_LOGI(MESH_TAG,"MESH_LEAF");
	xTaskCreate(leaf_send_task, "leaf_send_task", 4096, NULL, 10, NULL);
	xTaskCreate(leaf_recv_task, "leaf_recv_task", 4096, NULL, 10, NULL);
#endif

    /* mesh softAP */
    ESP_ERROR_CHECK(esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE));
    cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
    memcpy((uint8_t *) &cfg.mesh_ap.password, CONFIG_MESH_AP_PASSWD,
           strlen(CONFIG_MESH_AP_PASSWD));
    ESP_ERROR_CHECK(esp_mesh_set_config(&cfg));
    /* mesh start */
    ESP_ERROR_CHECK(esp_mesh_start());
    ESP_LOGI(MESH_TAG, "mesh starts successfully, heap:%d\n",  esp_get_free_heap_size());
}
