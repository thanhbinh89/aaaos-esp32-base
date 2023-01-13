#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_ui.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define TAG "TaskNet"

static int wifiRetryNum = 0;
static bool isGotIp = false;
static char ipString[16];

static void netEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
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

static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	switch (event_id) {
	case IP_EVENT_STA_GOT_IP: {
		APP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		sprintf(ipString, IPSTR, IP2STR(&event->ip_info.ip));
		APP_LOGI(TAG, "Got ip: %s", ipString);		
		wifiRetryNum = 0;
		isGotIp = true;
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_CONNECT_PERFORM, NULL, 0);
	}
		break;

	case IP_EVENT_STA_LOST_IP: {
		APP_LOGI(TAG, "IP_EVENT_STA_LOST_IP");
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

	esp_netif_create_default_wifi_sta();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netEventHandler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ipEventHandler, NULL);

	AAATaskPostMsg(AAA_TASK_NET_ID, NET_START_WIFI, NULL, 0);

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case NET_START_WIFI: {
			APP_LOGI(TAG, "NET_START_WIFI");

			wifi_config_t wifi_config = {
				.sta = {
					.ssid = "SSID",
					.password = "password",
					.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
				},
			};
			wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
			esp_wifi_set_mode(WIFI_MODE_STA);
			esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
			esp_wifi_start();
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