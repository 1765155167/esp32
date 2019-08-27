#include "esp_bt.h"

#include "mespnow.h"
#include "mconfig_blufi.h"
#include "mconfig_chain.h"

char WIFI_SSID[32] = {0};
char WIFI_PASSWD[64] = {0};
uint8_t MESH_ID[6] = {0x0};

static const char *TAG = "mconfig";

mdf_err_t get_network_config(const char *name, mwifi_config_t *mwifi_config, char custom_data[32])
{
    MDF_PARAM_CHECK(name);
    MDF_PARAM_CHECK(mwifi_config);
    MDF_PARAM_CHECK(custom_data);

    mconfig_data_t *mconfig_data        = NULL;
    mconfig_blufi_config_t blufi_config = {
        .tid = 1, /**< Type of device. Used to distinguish different products,
                       APP can display different icons according to this tid. */
        .company_id = MCOMMON_ESPRESSIF_ID, /**< Company Identifiers (https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers) */
    };

    strncpy(blufi_config.name, name, sizeof(blufi_config.name) - 1);
    MDF_LOGI("BLE name: %s", name);

    /**
     * @brief Initialize Bluetooth network configuration
     */
    MDF_ERROR_ASSERT(mconfig_blufi_init(&blufi_config));

    /**
     * @brief Network configuration chain slave initialization for obtaining network configuration information from master.
     */
    MDF_ERROR_ASSERT(mconfig_chain_slave_init());

    /**
     * @brief Get Network configuration information from blufi or network configuration chain.
     *      When blufi or network configuration chain complete, will send configuration information to config_queue.
     */
    MDF_ERROR_ASSERT(mconfig_queue_read(&mconfig_data, portMAX_DELAY));

    /**
     * @brief Deinitialize Bluetooth network configuration and Network configuration chain.
     */
    MDF_ERROR_ASSERT(mconfig_chain_slave_deinit());
    MDF_ERROR_ASSERT(mconfig_blufi_deinit());

    memcpy(mwifi_config, &mconfig_data->config, sizeof(mwifi_config_t));
    memcpy(custom_data, &mconfig_data->custom, sizeof(mconfig_data->custom));

    /**
     * @brief Switch to network configuration chain master mode to configure the network for other devices(slave), according to the white list.
     */
    if (mconfig_data->whitelist_size > 0) {
        MDF_ERROR_ASSERT(mconfig_chain_master(mconfig_data, pdMS_TO_TICKS(60000)));
    }

    MDF_FREE(mconfig_data);

    return MDF_OK;
}

esp_err_t get_wifi_info(mwifi_config_t * mwifi_config)
{
	esp_err_t err;
	size_t size_ssid = 32;
	size_t size_passwd = 64;
	size_t size_meshid = 6;
	nvs_handle wifi_handle;
	char name[28]        = {0x0};
    char custom_data[32] = {0x0};
	ESP_LOGI(TAG,"START GET WIFI INFO...");
	err = nvs_open("WIFI", NVS_READWRITE, &wifi_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		ESP_LOGI(TAG,"TRY LOAD WIFI INFO");
		err = nvs_get_str(wifi_handle, "ssid", WIFI_SSID, &size_ssid);
		if(err != ESP_OK) goto NVSFAL;
		memcpy(mwifi_config->router_ssid, WIFI_SSID, sizeof(WIFI_SSID));
		
		
		err = nvs_get_str(wifi_handle, "passwd", WIFI_PASSWD, &size_passwd);
		if(err != ESP_OK) goto NVSFAL;
		memcpy(mwifi_config->router_password, WIFI_PASSWD, sizeof(WIFI_PASSWD));
		
		err = nvs_get_blob(wifi_handle, "meshid", (void *)MESH_ID, &size_meshid);
		if(err != ESP_OK) goto NVSFAL;
		memcpy(mwifi_config->mesh_id, MESH_ID, sizeof(MESH_ID));

		err = nvs_commit(wifi_handle);
		if(err != ESP_OK)
		{
NVSFAL:
			ESP_LOGI(TAG,"WIFI SSID PASSWD LOAD ERR");
			/**
			 * @brief   1.Initialize event loop, receive event
			 *          2.Initialize wifi with station mode
			 *          3.Initialize espnow(ESP-NOW is a kind of connectionless WiFi communication protocol)
			 */
			MDF_ERROR_ASSERT(mespnow_init());

			uint8_t sta_mac[6] = {0};
			MDF_ERROR_ASSERT(esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac));
			sprintf(name, "ESP-MESH_%02x%02x", sta_mac[4], sta_mac[5]);

			/**
			 * @note `custom_data` is used for specific application custom data, non-essential fields.
			 *       can be passed through the APP distribution network (ESP-MESH Config (The bottom
			 *       right corner of the configured page) > Custom Data)
			 */
			MDF_ERROR_ASSERT(get_network_config(name, mwifi_config, custom_data));

			MDF_LOGI("mconfig, ssid: %s, password: %s, mesh_id: " MACSTR ", custom: %.*s",
					mwifi_config->router_ssid, mwifi_config->router_password,
					MAC2STR(mwifi_config->mesh_id), sizeof(custom_data), custom_data);

			/**
			 * @brief Note that once BT controller memory is released, the process cannot be reversed.
			 *        It means you can not use the bluetooth mode which you have released by this function.
			 *        it can release the .bss, .data and other section to heap
			 */
			MDF_ERROR_ASSERT(esp_bt_mem_release(ESP_BT_MODE_BLE));

			err = nvs_open("WIFI", NVS_READWRITE, &wifi_handle);
			if (err != ESP_OK) {
				printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
			} else {
				err = nvs_set_str(wifi_handle, "ssid", (char *)mwifi_config->router_ssid);
				err = nvs_set_str(wifi_handle, "passwd", (char *)mwifi_config->router_password);
				err = nvs_set_blob(wifi_handle, "meshid", (void *)mwifi_config->mesh_id,sizeof(mwifi_config->mesh_id));
				err = nvs_commit(wifi_handle);
				printf((err != ESP_OK) ? "WIFI SSID PASSWD SAVE ERR\n" : "WIFI SSID PASSWD SAVE OK\n");
				nvs_close(wifi_handle);// Close
			}
			nvs_close(wifi_handle);// Close
		}else
		{
			ESP_LOGI(TAG,"WIFI SSID PASSWD LOAD OK");
		}
	}
	nvs_close(wifi_handle);// Close
	return err;
}

esp_err_t erase_wifi_info(void)
{
	esp_err_t err;
	nvs_handle wifi_handle;
	err = nvs_open("WIFI", NVS_READWRITE, &wifi_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		err = nvs_erase_all(wifi_handle);
		if(err != ESP_OK)
		{
			ESP_LOGW(TAG,"wifi erase error.");
		}else{
			ESP_LOGI(TAG,"wifi erase success.");
		}
	}
	nvs_close(wifi_handle);// Close
	return err;
}
