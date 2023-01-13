#include "aaa.h"

#include "app.h"
#include "app_log.h"

#include "sys_io.h"

#define MAC_FORMAT "%02x%02x%02x%02x%02x%02x"
#define MAC_SPLIT(m) m[0], m[1], m[2], m[3], m[4], m[5]

#define TAG "App"

char gMacAddrStr[13] = { 0 };

void AAATaskInit() {
	APP_LOGI(TAG, "App version: " APP_FW_VERSION);
#ifdef RELEASE
	APP_LOGW(TAG, "Build type: RELEASE");
#else
	APP_LOGI(TAG, "Build type: DEBUG");
#endif

	platformInit();

	//Get MAC address
	uint8_t macAddr[6];
	getMacAddress(macAddr);
	sprintf(gMacAddrStr, MAC_FORMAT, MAC_SPLIT(macAddr));
	APP_LOGI(TAG, "MAC address: %s", gMacAddrStr);
}
