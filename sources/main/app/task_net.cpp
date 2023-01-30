#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_ui.h"

#include "esp_eth.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_err.h"

#define TAG "TaskNet"

static int wifiRetryNum = 0;
static bool isGotIp = false;
static char ipString[16];
static char maskString[16];
static char gwString[16];
static esp_eth_handle_t eth_handle = NULL;

static bool getNetType(int8_t *type);

static void netWifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	static int retry_num = 0;

	switch (event_id) {
	case WIFI_EVENT_STA_START: {
		APP_LOGI(TAG, "WIFI_EVENT_STA_START");
		esp_wifi_connect();
	}
		break;

	case WIFI_EVENT_STA_DISCONNECTED: {
		APP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
		if (retry_num++ < 300) {
			esp_wifi_connect();
			APP_LOGD(TAG, "retry to connect to the AP");
		} 
		else {
			//TODO: 
		}
	}
		break;
	
	default:
		break;
	}
}

static void netEthEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	switch (event_id) {
	case ETHERNET_EVENT_CONNECTED: {
		APP_LOGI(TAG, "ETHERNET_EVENT_CONNECTED");
		uint8_t mac[6] = {0};
		esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;
		esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac);
		APP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        APP_LOGI(TAG, "ETHERNET_EVENT_DISCONNECTED");
        break;
    case ETHERNET_EVENT_START:
        APP_LOGI(TAG, "ETHERNET_EVENT_START");
        break;
    case ETHERNET_EVENT_STOP:
        APP_LOGI(TAG, "ETHERNET_EVENT_STOP");
        break;
	
	default:
		break;
	}
}

static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	switch (event_id) {
	case IP_EVENT_STA_GOT_IP: 
	case IP_EVENT_ETH_GOT_IP: {
		APP_LOGI(TAG, "IP_EVENT_STA_GOT_IP|IP_EVENT_ETH_GOT_IP");
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		sprintf(ipString, IPSTR, IP2STR(&event->ip_info.ip));
		sprintf(maskString, IPSTR, IP2STR(&event->ip_info.netmask));
		sprintf(gwString, IPSTR, IP2STR(&event->ip_info.gw));
		APP_LOGI(TAG, "Got ip: %s, mask: %s, gw: %s", ipString, maskString, gwString);		
		wifiRetryNum = 0;
		isGotIp = true;
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_CONNECT_PERFORM, NULL, 0);
	}
		break;

	case IP_EVENT_STA_LOST_IP: 
	case IP_EVENT_ETH_LOST_IP: {
		APP_LOGI(TAG, "IP_EVENT_STA_LOST_IP|IP_EVENT_ETH_LOST_IP");
		isGotIp = false;
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_FORCE_DISCONNECT, NULL, 0);
	}
	
	default:
		break;
	}
}

void TaskNetEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	esp_netif_init();
	esp_event_loop_create_default();

	int8_t netType;
	if (getNetType(&netType)) {	
		APP_LOGI(TAG, "get net type: %d", netType);

		switch (netType) {
		case NET_TYPE_INT_ETH: {
			ethIoInit();
			vTaskDelay(3000);	

			// Init MAC and PHY configs to default
			eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
			eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

			phy_config.phy_addr = 0;
			phy_config.reset_gpio_num = GPIO_ETH_RST;
			eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
			esp32_emac_config.smi_mdc_gpio_num = GPIO_ETH_MDC;
			esp32_emac_config.smi_mdio_gpio_num = GPIO_ETH_MDIO;
			esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);
			esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);			

			esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);			
			esp_eth_driver_install(&config, &eth_handle);
			
			/* attach Ethernet driver to TCP/IP stack */
			// Create new default instance of esp-netif for Ethernet
			esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
			esp_netif_t *eth_netif = esp_netif_new(&cfg);
			esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle));
			
			esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &netEthEventHandler, NULL);
			esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &ipEventHandler, NULL);
			esp_eth_start(eth_handle);
		}			
			break;

		case NET_TYPE_SPI_ETH:
			APP_LOGI(TAG, "Unimplement!!!");
			break;

		case NET_TYPE_WIFI: {
			esp_netif_create_default_wifi_sta();
			wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
			esp_wifi_init(&cfg);

			esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netWifiEventHandler, NULL);
			esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ipEventHandler, NULL);

			esp_wifi_set_mode(WIFI_MODE_STA);
			esp_wifi_start();

			AAATaskPostMsg(AAA_TASK_NET_ID, NET_START_WIFI, NULL, 0);
		}			
			break;
		
		default:
			break;
		}		
	}

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case NET_START_WIFI: {
			APP_LOGI(TAG, "NET_START_WIFI");

			nvs_handle_t handle;
			esp_err_t err;
			wifi_config_t wifi_config;
			bzero(&wifi_config, sizeof(wifi_config));
			wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
			wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;

			// Get wifi infos in flash
			size_t len;
			// char wifi_config.sta.ssid[32], wifi_config.sta.password[64];
			err = nvs_open(NVS_WIFI_INFO, NVS_READONLY, &handle);
			if (err != ESP_OK) break;
			err = nvs_get_blob(handle, "ssid", wifi_config.sta.ssid, &len);
			wifi_config.sta.ssid[len] = 0;
			if (err != ESP_OK) break;
			err = nvs_get_blob(handle, "psk", wifi_config.sta.password, &len);
			wifi_config.sta.password[len] = 0;
			if (err != ESP_OK) break;
			nvs_close(handle);

			if (err == ESP_OK) {
				APP_LOGI(TAG, "conecting to ssid: %s, psk: %s\n", (char *)wifi_config.sta.ssid, (char *)wifi_config.sta.password);
				esp_wifi_disconnect();
				esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
				esp_wifi_connect();
			}
		}
			break;

		default:
			break;
		}
		AAAFreeMsg(msg);
	}
}

bool gotIP(char **ipStr) {
	if (ipStr) {
		*ipStr = ipString;
	}
	return isGotIp;
}

bool getNetType(int8_t *type) {
	nvs_handle_t handle;
	esp_err_t err;

	// Open
	err = nvs_open(NVS_NET_CFG, NVS_READWRITE, &handle);
	if (err != ESP_OK) return false;
	err = nvs_get_i8(handle, "type", type);
	// Close
	nvs_close(handle);
	return err == ESP_OK;
}