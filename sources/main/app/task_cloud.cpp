#include <time.h>
#include <string.h>

#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_cloud.h"
#include "task_net.h"
#include "MqttConnection.h"

#include "utils.h"

#include "sys_io.h"

#define TAG "TaskCloud"

void MqttConnection::onConnect(int rc) {
	if (rc == 0) {
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_ON_CONNECTED, NULL, 0);
	}
}

static void parserIncomingMessage(MqttConnection::DataItem_t *incomItem) {
	APP_LOGD(TAG, "Parser topic: %s, data length: %d", incomItem->topic, incomItem->dataLen);
	APP_LOGD(TAG, "%s", incomItem->data);

	if (strstr(incomItem->topic, SERVER_ATTRIBUTE_SUB_TOPIC)) {
		if (!strstr(incomItem->data, "\"deleted\"") && strstr(incomItem->data, "\"fw_title\"")) {
			FOTAAssign_t FOTAAssign;
			memset(&FOTAAssign, 0, sizeof(FOTAAssign));
			if (strstr(incomItem->data, "\"fw_url\"")) {
				getJsonString(incomItem->data, FOTAAssign.url, "fw_url");
				APP_LOGD(TAG, "URL: %s", FOTAAssign.url);
			}
			else {								
				getJsonString(incomItem->data, FOTAAssign.title, "fw_title");
				getJsonString(incomItem->data, FOTAAssign.version, "fw_version");
				getJsonString(incomItem->data, FOTAAssign.MD5Checksum, "fw_checksum");
				getJsonNumber(incomItem->data, &FOTAAssign.size, "fw_size", false);
				memset(FOTAAssign.url, 0, sizeof(FOTAAssign.url));
				APP_LOGD(TAG, "Target title: %s", FOTAAssign.title);
				APP_LOGD(TAG, "Target version: %s", FOTAAssign.version);
				APP_LOGD(TAG, "MD5Checksum: %s", FOTAAssign.MD5Checksum);
				APP_LOGD(TAG, "Size: %ld", FOTAAssign.size);
			}
			AAATaskPostMsg(AAA_TASK_OTA_ID, OTA_START, &FOTAAssign, sizeof(FOTAAssign));
		}
	}
	else if (strstr(incomItem->topic, SERVER_RPC_SUB_PREFIX_TOPIC)) {
		/* {"method":"OUTPUT0","params":{"value":1}} */
		char stringValue[32];
		uint32_t numberValue;
		bool found = getJsonString(incomItem->data, stringValue, "method");
		if (found && (0 == strcmp(stringValue, "OUTPUT0"))) {
			bool found = getJsonNumber(incomItem->data, &numberValue, "value", true);
			if (found) {
				setOutput0(numberValue);
			}
		}
		else if (found && (0 == strcmp(stringValue, "OUTPUT1"))) {
			bool found = getJsonNumber(incomItem->data, &numberValue, "value", true);
			if (found) {
				setOutput1(numberValue);
			}
		}

		char *reqId = incomItem->topic + (int) strlen(SERVER_RPC_SUB_PREFIX_TOPIC);
		char *topic = (char*) pvPortMalloc(strlen(SERVER_RPC_PUB_PREFIX_TOPIC) + strlen(reqId) + 1);
		int topicLen = sprintf(topic, "%s%s", SERVER_RPC_PUB_PREFIX_TOPIC, reqId);
		char *data = (char*) pvPortMalloc(64);
		int dataLen = sprintf(data, "{\"response\":{\"uptime\":%ld}}", (uint32_t)time(0));
		MqttConnection::DataItem_t sendItem;
		sendItem.topic = topic;
		sendItem.topicLen = topicLen;
		sendItem.data = data;
		sendItem.dataLen = dataLen;
		sendItem.inHeap = true;
		Mqtt.sendEnqueue(&sendItem);
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
	}
}

void TaskCloudEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	Mqtt.initialize(SERVER_MQTT_HOST, SERVER_MQTT_PORT, (const char*) gMacAddrStr, (const char*) gMacAddrStr);

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case CLOUD_MQTT_CONNECT_PERFORM: {
			APP_LOGI(TAG, "CLOUD_MQTT_CONNECT_PERFORM");
			Mqtt.connect();
		}
			break;

		case CLOUD_MQTT_FORCE_DISCONNECT: {
			APP_LOGI(TAG, "CLOUD_MQTT_FORCE_DISCONNECT");			
			Mqtt.disconnect();
		}
			break;

		case CLOUD_MQTT_ON_CONNECTED: {
			APP_LOGI(TAG, "CLOUD_MQTT_ON_CONNECTED");
			static char buf[32];
			char *ips;
			gotIP(&ips);
			int len = sprintf(buf, "{\"IP\":\"%s\"}", ips);
			if (!Mqtt.sendQueueIsFull()) {
				MqttConnection::DataItem_t sendItem;
				sendItem.topic = (char*) SERVER_ATTRIBUTE_PUB_TOPIC;
				sendItem.topicLen = strlen(sendItem.topic);
				sendItem.data = (char*) buf;
				sendItem.dataLen = len;
				sendItem.inHeap = false;
				Mqtt.sendEnqueue(&sendItem);

				sendItem.topic = (char*) SERVER_TELEMETRY_PUB_TOPIC;
				sendItem.topicLen = strlen(sendItem.topic);
				sendItem.data = (char*) "{\"current_fw_title\":\"" APP_FW_TITLE "\",\"current_fw_version\":\"" APP_FW_VERSION "\"}";
				sendItem.dataLen = strlen(sendItem.data);
				sendItem.inHeap = false;
				Mqtt.sendEnqueue(&sendItem);

				AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
			}
			Mqtt.subscribe(SERVER_ATTRIBUTE_SUB_TOPIC, 1);
			Mqtt.subscribe(SERVER_RPC_SUB_TOPIC, 1);
		}
			break;

		case CLOUD_MQTT_ON_MESSAGE: {
			APP_LOGI(TAG, "CLOUD_MQTT_ON_MESSAGE");
			MqttConnection::DataItem_t incomItem;
			while (Mqtt.incomQueueIsAvailable()) {
				Mqtt.incomDequeue(&incomItem);
				parserIncomingMessage(&incomItem);
				if (incomItem.inHeap) {
					Mqtt.freeQueueItem(&incomItem);
				}
			}
		}
			break;

		case CLOUD_MQTT_SEND_QUEUE: {
			APP_LOGI(TAG, "CLOUD_MQTT_SEND_QUEUE");
			if (gotIP(NULL) && Mqtt.isConnected() && Mqtt.sendQueueIsAvailable()) {			
				Mqtt.publishQueue();
			}
		}
			break;

		default:
			break;
		}

		AAAFreeMsg(msg);
	}
}

void sendStaticMsgToServer(const char *topic, const char *data) {
	if (!Mqtt.sendQueueIsFull()) {
		MqttConnection::DataItem_t sendItem;
		sendItem.topic = (char*) topic;
		sendItem.topicLen = strlen(sendItem.topic);
		sendItem.data = (char*) data;
		sendItem.dataLen = strlen(sendItem.data);
		sendItem.inHeap = false;
		Mqtt.sendEnqueue(&sendItem);
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
	}
}

void sendDynamicMsgToServer(char *topic, char *data, int dataLen) {
	if (!Mqtt.sendQueueIsFull()) {
		MqttConnection::DataItem_t sendItem;
		sendItem.topic = topic;
		sendItem.topicLen = strlen(sendItem.topic);
		sendItem.data = data;
		sendItem.dataLen = dataLen;
		sendItem.inHeap = true;
		Mqtt.sendEnqueue(&sendItem);
		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
	}
}

