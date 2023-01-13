#include "aaa.h"
#include "aaa_log.h"

#include "task_list.h"

typedef struct tAAATimerArg {
	uint32_t tId;
	uint32_t sig;
	bool reload;
	uint32_t len;
	void *data;
} AAATimerArg_t;

#define TAG "AAA"

static SemaphoreHandle_t SemTasksStarted = NULL;
static uint32_t TaskTableLen = AAA_TASK_LIST_LEN;

static void printBanner(void) {
	printf("\r\n"
			"      _/_/      _/_/      _/_/      _/_/      _/_/_/   \r\n"
			"   _/    _/  _/    _/  _/    _/  _/    _/  _/          \r\n"
			"  _/_/_/_/  _/_/_/_/  _/_/_/_/  _/    _/    _/_/       \r\n"
			" _/    _/  _/    _/  _/    _/  _/    _/        _/      \r\n"
			"_/    _/  _/    _/  _/    _/    _/_/    _/_/_/         \r\n"
			"Author: thanhbinh89\r\n"
			"Build: %s\r\n",
			__DATE__);
}

void app_main(void) {
	printBanner();
	AAA_LOGD(TAG, "TaskTableLen: %ld", TaskTableLen);

	AAATaskInit();

	SemTasksStarted = xSemaphoreCreateCounting(TaskTableLen, TaskTableLen);
	SYS_ASSERT(SemTasksStarted != NULL);

	for (uint32_t idx = 0; idx < TaskTableLen; idx++) {
		TaskList[idx].qHandle = xQueueCreate(AAA_TASK_QUEUE_LENGTH,
				sizeof(AAAMsg_t));
		SYS_ASSERT(TaskList[idx].qHandle != NULL);

		SYS_ASSERT(pdFAIL != xTaskCreate(TaskList[idx].tFunc, TaskList[idx].tDesc, TaskList[idx].tDepth, &TaskList[idx].tId, TaskList[idx].tPriority, NULL));

		AAA_LOGD(TAG, "CreateTask desc: %s, id: %ld, depth: %d, prio: %d, entry: %p",
				TaskList[idx].tDesc,
				TaskList[idx].tId,
				TaskList[idx].tDepth,
				TaskList[idx].tPriority,
				TaskList[idx].tFunc);
	}
}

void AAAWaitAllTaskStarted(void) {
	AAA_LOGD(TAG, "TaskWait ...");

	SYS_ASSERT(pdFAIL != xSemaphoreTake(SemTasksStarted, portMAX_DELAY));

	while (uxSemaphoreGetCount(SemTasksStarted) > 0) {
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

bool AAATaskPostMsg(uint32_t tId, uint32_t sig, void *msg, uint32_t len) {
	AAA_LOGD(TAG, "TaskPost id:%ld, sig:%ld, msg:%p, len:%ld",
			tId,
			sig,
			msg,
			len);

	AAAMsg_t aaaMsg = { sig, len, NULL };

	SYS_ASSERT(tId < TaskTableLen);

	if (len) {
		aaaMsg.data = pvPortMalloc(len);
		SYS_ASSERT(aaaMsg.data != NULL);

		memcpy(aaaMsg.data, msg, len);
		AAA_LOGD(TAG, "TaskPost copy msg %p -> %p, len:%ld",
				msg,
				aaaMsg.data,
				len);
	}

	SYS_ASSERT(pdFAIL != xQueueSend(TaskList[tId].qHandle, &aaaMsg, portMAX_DELAY));
	return true;
}

bool AAATaskPostMsgFromISR(uint32_t tId, uint32_t sig, void *msg, uint32_t len) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	AAAMsg_t aaaMsg = { sig, len, NULL };

	if (tId >= TaskTableLen)
		return false;

	if (len) {
		aaaMsg.data = pvPortMalloc(len);
		if (aaaMsg.data == NULL)
			return false;

		memcpy(aaaMsg.data, msg, len);
	}

	if (pdTRUE != xQueueSendFromISR(TaskList[tId].qHandle, &aaaMsg,
					&xHigherPriorityTaskWoken))
		return false;
	if (xHigherPriorityTaskWoken == pdTRUE) {
		portYIELD();
	}
	return true;
}

bool AAATaskRecvMsg(uint32_t tId, uint32_t *sig, void **msg, uint32_t *len) {
	SYS_ASSERT(tId < TaskTableLen);

	AAAMsg_t aaaMsg = { 0, 0, NULL };
	SYS_ASSERT(
			pdFAIL != xQueueReceive(TaskList[tId].qHandle, &aaaMsg, portMAX_DELAY));

	*sig = aaaMsg.sig;
	*msg = aaaMsg.data;
	*len = aaaMsg.len;
	if (msg == NULL) {
		;
	}
	AAA_LOGD(TAG, "%s, id:%ld, sig:%ld, msg:%p, len:%ld", __func__, tId, *sig, *msg, *len);

	return true;
}

void AAAFreeMsg(void *msg) {
	if (msg != NULL) {
		vPortFree(msg);
		AAA_LOGD(TAG, "Task msg free: %p", msg);
	}
}

static void prvAAATimerCallback(TimerHandle_t timHandler) {
	SYS_ASSERT(timHandler != NULL);

	AAATimerArg_t *aaaTimArg = (AAATimerArg_t*) pvTimerGetTimerID(timHandler);
	AAATaskPostMsg(aaaTimArg->tId, aaaTimArg->sig, aaaTimArg->data,
			aaaTimArg->len);
	if (!aaaTimArg->reload) {
		if (aaaTimArg->data != NULL) {
			vPortFree(aaaTimArg->data);
			AAA_LOGD(TAG, "Timer msg free: %p", aaaTimArg->data);
			aaaTimArg->data = NULL;
		}
		if (aaaTimArg != NULL) {
			vPortFree(aaaTimArg);
			aaaTimArg = NULL;
		}
	}
}

AAATimer_t AAATimerSet(uint32_t tId, uint32_t sig, void *msg, uint32_t len,
		uint32_t period, bool reload) {
	AAA_LOGD(TAG, "TimerSet id:%ld, sig:%ld, msg:%p, len:%ld, period:%ld, reload:%s",
			tId,
			sig,
			msg,
			len,
			period,
			reload ? "true" : "false");

	AAATimerArg_t *aaaTimArg = NULL;

	SYS_ASSERT(tId < TaskTableLen);

	aaaTimArg = (AAATimerArg_t*) pvPortMalloc(sizeof(AAATimerArg_t));
	SYS_ASSERT(aaaTimArg != NULL);

	aaaTimArg->tId = tId;
	aaaTimArg->sig = sig;
	aaaTimArg->reload = reload;
	aaaTimArg->len = len;
	aaaTimArg->data = NULL;

	if (aaaTimArg->len) {
		aaaTimArg->data = pvPortMalloc(aaaTimArg->len);
		SYS_ASSERT(aaaTimArg->data != NULL);

		memcpy(aaaTimArg->data, msg, aaaTimArg->len);
		AAA_LOGD(TAG, "TimerSet copy msg %p -> %p, len:%ld",
				msg,
				aaaTimArg->data,
				aaaTimArg->len);
	}

	TimerHandle_t timHandler = xTimerCreate("AAATim", pdMS_TO_TICKS(period),
			aaaTimArg->reload ? pdTRUE : pdFALSE, (void*) aaaTimArg,
			prvAAATimerCallback);
	if (timHandler != NULL) {
		xTimerStart(timHandler, pdMS_TO_TICKS(100));
		return (AAATimer_t) timHandler;
	}
	return (AAATimer_t) NULL;
}

void AAATimerRemove(AAATimer_t timer) {
	TimerHandle_t timHandler = timer;
	AAATimerArg_t *aaaTimArg = (AAATimerArg_t*) pvTimerGetTimerID(timHandler);
	AAA_LOGD(TAG, "TimerRemove: %p, id:%ld, sig:%ld",
			timer,
			aaaTimArg->tId,
			aaaTimArg->sig);

	xTimerDelete(timHandler, pdMS_TO_TICKS(100));
	if (aaaTimArg->data != NULL) {
		vPortFree(aaaTimArg->data);
		AAA_LOGD(TAG, "Timer msg free: %p", aaaTimArg->data);
		aaaTimArg->data = NULL;
	}
	if (aaaTimArg != NULL) {
		vPortFree(aaaTimArg);
		aaaTimArg = NULL;
	}
}
