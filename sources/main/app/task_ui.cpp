#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_ui.h"

#define TAG "TaskUi"

void TaskUiEntry(void *params) {
	AAAWaitAllTaskStarted();
	APP_LOGI(TAG, "started");

	void *msg = NULL;
	uint32_t len = 0, sig = 0;
	uint32_t id = *(uint32_t*) params;

	while (AAATaskRecvMsg(id, &sig, &msg, &len)) {
		switch (sig) {
		case UI_UPDATE: {
			APP_LOGI(TAG, "UI_UPDATE");
		}
			break;

		default:
			break;
		}
		AAAFreeMsg(msg);
	}
}

