#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_device.h"
#include "MqttConnection.h"

#include "sys_io.h"

#include "driver/gpio.h"

#define TAG "TaskDevice"

#define ESP_INTR_FLAG_DEFAULT	(0)

typedef struct t_input {
	gpio_num_t num;
	int value;
} input_t;

static void sendInput(input_t *input);

static void IRAM_ATTR gpio_isr_handler(void* arg) {
	gpio_num_t *num = (gpio_num_t *) arg;
	input_t input = {
		.num = *num, 
		.value = gpio_get_level(*num)
	};    
    AAATaskPostMsg(AAA_TASK_DEVICE_ID, DEVICE_INPUT_CHANGE, &input, sizeof(input));
}

void TaskDeviceEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	outputInit();
	inputInit();	

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
	static gpio_num_t input0 = GPIO_INPUT_IO_0;
    gpio_isr_handler_add(input0, gpio_isr_handler, (void*) &input0);
    //hook isr handler for specific gpio pin
	static gpio_num_t input1 = GPIO_INPUT_IO_1;
    gpio_isr_handler_add(input1, gpio_isr_handler, (void*) &input1);

	AAATimerSet(AAA_TASK_DEVICE_ID, DEVICE_TOGGLE_LED_LIFE, NULL, 0, DEVICE_TOGGLE_LED_LIFE_INTERVAL, true);
	AAATimerSet(AAA_TASK_DEVICE_ID, DEVICE_INPUT_SYNC, NULL, 0, DEVICE_INPUT_SYNC_INTERVAL, false);

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case DEVICE_TOGGLE_LED_LIFE:
			toggleLedLife();
			break;

		case DEVICE_INPUT_SYNC: {
			APP_LOGI(TAG, "DEVICE_INPUT_SYNC");
			input_t input;
			input.num = GPIO_INPUT_IO_0;
			input.value = gpio_get_level(GPIO_INPUT_IO_0);	 
			sendInput(&input);

			input.num = GPIO_INPUT_IO_1;
			input.value = gpio_get_level(GPIO_INPUT_IO_1);	 
			sendInput(&input);			
		}
			break;

		case DEVICE_INPUT_CHANGE: {
			APP_LOGI(TAG, "DEVICE_INPUT_CHANGE");
			input_t *input = (input_t *)msg;
			sendInput(input);			
		}
			break;

		default:
			break;
		}

		AAAFreeMsg(msg);
	}
}

void sendInput(input_t *input) {
	if (!Mqtt.sendQueueIsFull()) {
		MqttConnection::DataItem_t sendItem;				
		sendItem.topic = (char*) SERVER_TELEMETRY_PUB_TOPIC;
		sendItem.topicLen = strlen(sendItem.topic);
		if (input->num == GPIO_INPUT_IO_0) {					
			sendItem.data = input->value ? (char*) "{\"Input0\":1}" : (char*) "{\"Input0\":0}";
		}
		else {
			sendItem.data = input->value ? (char*) "{\"Input1\":1}" : (char*) "{\"Input1\":0}";
		}
		sendItem.dataLen = strlen(sendItem.data);
		sendItem.inHeap = false;
		Mqtt.sendEnqueue(&sendItem);

		AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
	}
}