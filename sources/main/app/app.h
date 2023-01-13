#ifndef __APP_H__
#define __APP_H__

#include "aaa.h"

/*****************************************************************************/
/*  task AAA_TASK_CONSOLE define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	CONSOLE_SEND_TEST,
};

/*****************************************************************************/
/*  task AAA_TASK_DEVICE define
 */
/*****************************************************************************/
/* define timer */
#define DEVICE_TOGGLE_LED_LIFE_INTERVAL		(1000)
#define DEVICE_INPUT_SYNC_INTERVAL			(1500)
/* define signal */
enum {
	DEVICE_TOGGLE_LED_LIFE = AAA_USER_DEFINE_SIG,
	DEVICE_INPUT_SYNC,
	DEVICE_INPUT_CHANGE,
};

/*****************************************************************************/
/*  task AAA_TASK_CLOUD define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	CLOUD_MQTT_CONNECT_PERFORM = AAA_USER_DEFINE_SIG,
	CLOUD_MQTT_FORCE_DISCONNECT,
	CLOUD_MQTT_ON_CONNECTED,	
	CLOUD_MQTT_ON_MESSAGE,
	CLOUD_MQTT_SEND_QUEUE,
};

/*****************************************************************************/
/*  task AAA_TASK_OTA define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	OTA_INIT,
	OTA_START,
	OTA_START_DOWNLOAD,
	OTA_DOWNLOADING,
	OTA_DONE_DOWNLOAD,
	OTA_DONE
};

/*****************************************************************************/
/*  task AAA_TASK_UI define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	UI_UPDATE,
};

/*****************************************************************************/
/*  task AAA_TASK_NET define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
enum {
	NET_START_WIFI,
};

/*****************************************************************************/
/*  global define variable
 */
/*****************************************************************************/
#define APP_FW_MODEL "AAAOS-ESP32-BASE"
#define APP_FW_VERSION "0.0.2"
#ifdef DEBUG
#define APP_FW_TITLE APP_FW_MODEL"-DEBUG"
#else
#define APP_FW_TITLE APP_FW_MODEL"-PROD"
#endif

#define TB_SERVER "broker.hivemq.com"

#define SERVER_MQTT_HOST TB_SERVER
#define SERVER_MQTT_PORT 1883
#define SERVER_TELEMETRY_PUB_TOPIC	"v1/devices/me/telemetry"
#define SERVER_ATTRIBUTE_PUB_TOPIC	"v1/devices/me/attributes"
#define SERVER_ATTRIBUTE_SUB_TOPIC	"v1/devices/me/attributes"
#define SERVER_RPC_SUB_PREFIX_TOPIC	"v1/devices/me/rpc/request/"
#define SERVER_RPC_SUB_TOPIC		SERVER_RPC_SUB_PREFIX_TOPIC "+"
#define SERVER_RPC_PUB_PREFIX_TOPIC	"v1/devices/me/rpc/response/"

#define HTTP_OTA_URI_FORMAT TB_SERVER "/api/v1/%s/firmware?title=%s&version=%s"

typedef struct tFOTAAssign {
	char title[32];
	char version[32];
	uint32_t size;
	char MD5Checksum[32];
	char url[256];
} FOTAAssign_t;

extern char gMacAddrStr[13];

#endif // __APP_H__
