#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_console.h"
#include "MqttConnection.h"

#include "sys_io.h"

#include "driver/uart.h"

#define TAG "TaskConsole"

#define RX_BUFFER_SIZE					(256)
#define CONSOLE_BUFFER_SIZE				RX_BUFFER_SIZE
struct console_t {
	uint8_t rxChar;
	uint8_t index;
	uint8_t data[CONSOLE_BUFFER_SIZE];
} console;
static QueueHandle_t uartQueue = NULL;

static void uartFrameHandle(uint8_t *buffer, uint8_t size) {
	uint8_t c = *buffer;
	while (size--) {
		if (console.index < CONSOLE_BUFFER_SIZE - 1) {
			if (c == '\r' || c == '\n') {
				console.data[console.index] = c;
				console.data[console.index + 1] = 0;
				{
					/* Parser with table */
					switch (cmd_line_parser(lgn_cmd_table, console.data)) {
					case CMD_SUCCESS:
						break;

					case CMD_NOT_FOUND:
						if (console.data[0] != '\r' && console.data[0] != '\n') {
							SYS_PRINTF("cmd unknown\n");
						}
						break;

					case CMD_TOO_LONG:
						SYS_PRINTF("cmd too long\n");
						break;

					case CMD_TBL_NOT_FOUND:
						SYS_PRINTF("cmd table not found\n");
						break;

					default:
						SYS_PRINTF("cmd error\n");
						break;
					}
				}
				SYS_PRINTF("#\n");

				console.index = 0;
			}
			else if (c == 8) {
				if (console.index) {
					console.index--;
				}
			}
			else {
				console.data[console.index++] = c;
			}
		}
		else {
			SYS_PRINTF("\nerror: cmd too long, cmd size: %d, try again !\n", CONSOLE_BUFFER_SIZE);
			console.index = 0;
		}
		c = *(++buffer);
	}
	
}

static void uartEventTask(void *pvParameters) {
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) pvPortMalloc(RX_BUFFER_SIZE);
    while (1) {
        //Waiting for UART event.
        if(xQueueReceive(uartQueue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY);
					uartFrameHandle(dtmp, event.size);				
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    APP_LOGD(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_0);
                    xQueueReset(uartQueue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    APP_LOGD(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(UART_NUM_0);
                    xQueueReset(uartQueue);
                    break;
                //Others
                default:
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void TaskConsoleEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	/* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uartCfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
		.source_clk = UART_SCLK_DEFAULT		
    };
    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_0, RX_BUFFER_SIZE * 2, 0, 10, &uartQueue, 0);
    uart_param_config(UART_NUM_0, &uartCfg);
    
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);  
	
	xTaskCreate(uartEventTask, "uartEv", AAA_TASK_DEPTH_MEDIUM, NULL, AAA_TASK_PRIORITY_MAX, NULL);

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case CONSOLE_SEND_TEST: {
			APP_LOGI(TAG, "CONSOLE_SEND_TEST");
			MqttConnection::DataItem_t sendItem;
			sendItem.topic = (char *) SERVER_ATTRIBUTE_PUB_TOPIC;
			sendItem.topicLen = strlen(sendItem.topic);
			sendItem.data = (char *)"{\"Test\":true}";
			sendItem.dataLen = strlen(sendItem.data);
			sendItem.inHeap = false;
			Mqtt.sendEnqueue(&sendItem);

			AAATaskPostMsg(AAA_TASK_CLOUD_ID, CLOUD_MQTT_SEND_QUEUE, NULL, 0);
		}
			break;

		default:
			break;
		}
		AAAFreeMsg(msg);
	}
}

