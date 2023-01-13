#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"

#define TAG "TaskOta"

void TaskOtaEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case OTA_INIT: {
			APP_LOGI(TAG, "OTA_INIT");

		}
			break;

		case OTA_START: {
			APP_LOGI(TAG, "OTA_START");
			
			FOTAAssign_t *FOTAAssign = (FOTAAssign_t*) msg;
			if (!strlen(FOTAAssign->url)) {
				sprintf(FOTAAssign->url, HTTP_OTA_URI_FORMAT, gMacAddrStr, FOTAAssign->title, FOTAAssign->version);					
			}
			APP_LOGD(TAG, "URL: %s", FOTAAssign->url);
			//TODO: download firmware from url
		}
			break;

		case OTA_START_DOWNLOAD: {
			APP_LOGI(TAG, "OTA_START_DOWNLOAD");
			
		}
			break;

		case OTA_DOWNLOADING: {
			APP_LOGI(TAG, "OTA_DOWNLOADING");

		}
			break;

		case OTA_DONE_DOWNLOAD: {
			APP_LOGI(TAG, "OTA_DONE_DOWNLOAD");
		
		}
			break;

		case OTA_DONE: {
			APP_LOGI(TAG, "OTA_DONE");
			
		}
			break;

		default:
			break;
		}
		AAAFreeMsg(msg);
	}
}

