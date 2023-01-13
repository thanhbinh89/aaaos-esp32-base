#ifndef SRC_APP_MQTTCONNECTION_H_
#define SRC_APP_MQTTCONNECTION_H_

#include "aaa.h"

#include "app_log.h"

#include "mqtt_client.h"
#include "esp_err.h"

class MqttConnection {
public:
	typedef struct t_IncomItem {
		char *topic;
		int topicLen;
		char *data;
		int dataLen;
		bool inHeap;
	} DataItem_t;

	bool isPublishing;
	bool connected;

	MqttConnection();
	virtual ~MqttConnection();
	void initialize(const char *host, uint16_t port = 1880, const char *clientId = "_client", const char *clientUser = "_user");
	bool connect() {
		isPublishing = false;
		connected = false;
		return (ESP_OK == esp_mqtt_client_start(client));
	};
	void disconnect() {
		esp_mqtt_client_disconnect(client);
		esp_mqtt_client_stop(client);
		isPublishing = false;
		connected = false;
	};
	bool isConnected() {
		return connected;
	};
	void subscribe(const char *topic, int qos = 0);
	void publish(const char *topic, char *data, int dataLen, int qos = 0, int retain = 0);
	void publishQueue();
	bool incomQueueIsFull() {
		return (0 == (int) uxQueueSpacesAvailable(incomQueue));
	};
	int incomQueueIsAvailable() {
		return (int) uxQueueMessagesWaiting(incomQueue);
	};
	void incomEnqueue(DataItem_t *IncomItem) {
		xQueueSend(incomQueue, IncomItem, portMAX_DELAY);
	};
	void incomDequeue(DataItem_t *IncomItem) {
		xQueueReceive(incomQueue, IncomItem, portMAX_DELAY);
	};
	bool sendQueueIsFull() {
		return (0 == (int) uxQueueSpacesAvailable(sendQueue));
	};
	int sendQueueIsAvailable() {
		return (int) uxQueueMessagesWaiting(sendQueue);
	};
	void sendEnqueue(DataItem_t *SendItem) {
		xQueueSend(sendQueue, SendItem, portMAX_DELAY);
	};
	void sendPeekQueue(DataItem_t *SendItem) {
		xQueuePeek(sendQueue, SendItem, portMAX_DELAY);
	};
	void sendDequeue(DataItem_t *SendItem) {
		xQueueReceive(sendQueue, SendItem, portMAX_DELAY);
	};
	void freeQueueItem(DataItem_t *SendItem) {
		vPortFree(SendItem->topic);
		vPortFree(SendItem->data);
	};
	virtual void onConnect(int rc);

private:
	esp_mqtt_client_handle_t client;	
	QueueHandle_t incomQueue, sendQueue;
};

extern MqttConnection Mqtt;

/*
 * TELEMETRY
 * Topic
 *  v1/devices/me/telemetry
 * Data publish
 *  {"key1":"value1", "key2":"value2"} or
 *  [{"key1":"value1"}, {"key2":"value2"}] or
 *  {"ts":1451649600512, "values":{"key1":"value1", "key2":"value2"}}
 *
 * ATTRIBUTES *
 * Publish attribute update to the server
 *  v1/devices/me/attributes
 * Request attribute values from the server
 *  v1/devices/me/attributes/request/$request_id
 *  v1/devices/me/attributes/response/+
 * Subscribe to attribute updates from the server
 *  v1/devices/me/attributes
 *  {"key1":"value1"}
 *  {"deleted":["fw_checksum_algorithm","fw_version","fw_ts","fw_tag","fw_checksum","fw_title","fw_state"
,"fw_size","fw_url"]}
 *  {"fw_title":"ATM-MON-01-DEBUG","fw_version":"0.0.1","fw_tag":"ATM-MON-01-DEBUG 0.0.1","fw_size":20287
2,"fw_checksum_algorithm":"MD5","fw_checksum":"70ae53c30da61495b0c357f6a3f523c7"}
 *
 * RPC
 * Subscribe to RPC commands from the server
 *  v1/devices/me/rpc/request/$request_id
 *  v1/devices/me/rpc/response/$request_id
 * Send RPC commands to server
 *  v1/devices/me/rpc/request/$request_id
 *  v1/devices/me/rpc/response/$request_id
 *
 * */

#endif /* SRC_APP_MQTTCONNECTION_H_ */
