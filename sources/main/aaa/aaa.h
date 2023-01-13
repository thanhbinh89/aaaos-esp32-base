#ifndef __AAA_H__
#define __AAA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define AAA_VERSION "1.1"

#define AAA_TASK_QUEUE_LENGTH (16)

#define AAA_USER_DEFINE_SIG 10

/* Task depth in bytes = AAATaskDepth_t * sizeof( portSTACK_TYPE ) */
typedef enum eAAATaskDepth
{
	AAA_TASK_DEPTH_LOW = 2048,
	AAA_TASK_DEPTH_MEDIUM = 4096,
	AAA_TASK_DEPTH_HIGH = 8192,
	AAA_TASK_DEPTH_MAX = 16384,
} AAATaskDepth_t;

typedef enum eAAATaskPriority
{
	AAA_TASK_PRIORITY_LOW = 1,
	AAA_TASK_PRIORITY_NORMAL,
	AAA_TASK_PRIORITY_HIGH,
	AAA_TASK_PRIORITY_MAX,
} AAATaskPriority_t;

typedef struct tAAATask
{
	uint32_t tId;
	void (*tFunc)(void *);
	AAATaskDepth_t tDepth;
	AAATaskPriority_t tPriority;
	const char *const tDesc;
	void *qHandle;
} AAATask_t;

typedef struct tAAAMsg
{
	uint32_t sig;
	uint32_t len;
	void *data;
} AAAMsg_t;

typedef TimerHandle_t AAATimer_t;

/******************************************************************************
* task function
*******************************************************************************/
/* function is called to start aaaos */
// extern void AAAOS(void);

/* function is called before create threads */
extern void AAATaskInit(void);

/* function using to make sure that all task is initialed */
extern void AAAWaitAllTaskStarted(void);

/* function exchange messages */
extern bool AAATaskPostMsg(uint32_t, uint32_t, void *, uint32_t);
extern bool AAATaskPostMsgFromISR(uint32_t, uint32_t, void *, uint32_t);
extern bool AAATaskRecvMsg(uint32_t, uint32_t *, void **, uint32_t *);
extern void AAAFreeMsg(void *);

/* function timer */
extern AAATimer_t AAATimerSet(uint32_t tId, uint32_t sig, void *msg, uint32_t len, uint32_t period, bool reload);
extern void AAATimerRemove(AAATimer_t timer);

#ifdef __cplusplus
}
#endif

#endif
