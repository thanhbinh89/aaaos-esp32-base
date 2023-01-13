#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"

#include "MqttConnection.h"

#define TAG "MQTT"

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        APP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

extern "C" void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {    
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED: {
        APP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
		if (handler_args != NULL) {
			MqttConnection *mqtt = (MqttConnection*) handler_args;
			mqtt->connected = true;
			mqtt->onConnect(0);
		}
	}
        break;

    case MQTT_EVENT_DISCONNECTED: {
        APP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		MqttConnection *mqtt = (MqttConnection*) handler_args;
		mqtt->connected = false;
	}
        break;

    case MQTT_EVENT_SUBSCRIBED:
        APP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);     
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        APP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);		
        break;

    case MQTT_EVENT_PUBLISHED: {
        APP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		if (handler_args != NULL) {
			MqttConnection *mqtt = (MqttConnection*) handler_args;
			MqttConnection::DataItem_t item;
			mqtt->sendDequeue(&item);
			if (item.inHeap) {
				mqtt->freeQueueItem(&item);
			}
			mqtt->isPublishing = false;
			AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);			
		}
	}
        break;

    case MQTT_EVENT_DATA: {
        APP_LOGI(TAG, "MQTT_EVENT_DATA");
		if (handler_args != NULL) {
			MqttConnection *mqtt = (MqttConnection*) handler_args;

			if (!mqtt->incomQueueIsFull()) {
				MqttConnection::DataItem_t item;
				item.inHeap = true;

				item.topic = (char*) pvPortMalloc(event->topic_len + 1);
				if (item.topic) {
					memcpy(item.topic, event->topic, event->topic_len);
					item.topic[event->topic_len] = 0;
					item.topicLen = event->topic_len;

					item.data = (char*) pvPortMalloc(event->data_len + 1);
					if (item.data) {
						memcpy(item.data, event->data, event->data_len);
						item.data[event->data_len] = 0;
						item.dataLen = event->data_len;
						mqtt->incomEnqueue(&item);
						AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_ON_MESSAGE, NULL, 0);
					}
				}
			}
		}
	}
        break;

    case MQTT_EVENT_ERROR:
        APP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            APP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        APP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

MqttConnection::MqttConnection() {
	client = NULL;
	incomQueue = NULL;
	sendQueue = NULL;
	isPublishing = false;
	connected = false;
}

MqttConnection::~MqttConnection() { }

void MqttConnection::initialize(const char *host, uint16_t port, const char *clientId, const char *clientUser) {
	static esp_mqtt_client_config_t cfg;	
	
	cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
	cfg.broker.address.hostname = host;
	cfg.broker.address.port = port;
	cfg.credentials.username = clientUser;
	cfg.credentials.client_id = clientId;
	cfg.session.keepalive = 120;
	cfg.network.disable_auto_reconnect = false;	

	client = esp_mqtt_client_init(&cfg);
	if (client) {
		incomQueue = xQueueCreate(5, sizeof(DataItem_t));
		sendQueue = xQueueCreate(32, sizeof(DataItem_t));
		esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqttEventHandler, this);    	
	}
}

void MqttConnection::subscribe(const char *topic, int qos) {
	esp_mqtt_client_subscribe(client, topic, qos);
	APP_LOGD(TAG, "Subscribe topic: %s", topic);
}

void MqttConnection::publish(const char *topic, char *data, int dataLen, int qos, int retain) {
	esp_mqtt_client_publish(client, topic, (const char *)data, dataLen, qos, retain);
	APP_LOGD(TAG, "Publish topic: %s, data length: %d", topic, dataLen);
}

void MqttConnection::publishQueue() {
	if (!isPublishing) {
		isPublishing = true;
		DataItem_t item;
		sendPeekQueue(&item);
		publish(item.topic, item.data, item.dataLen, 1, 0);
	}
}

MqttConnection Mqtt;
