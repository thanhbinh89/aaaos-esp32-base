#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "aaa.h"

#include "task_console.h"
#include "task_device.h"
#include "task_cloud.h"
#include "task_ota.h"
#include "task_ui.h"
#include "task_net.h"

enum eAAATaskID
{
	/* APP TASKS */
	AAA_TASK_CONSOLE_ID,	
	AAA_TASK_DEVICE_ID,	
	AAA_TASK_CLOUD_ID,	
	AAA_TASK_OTA_ID,
	AAA_TASK_UI_ID,
	AAA_TASK_NET_ID,
	/* EOT task ID */
	AAA_TASK_LIST_LEN,
};

extern AAATask_t TaskList[];

#endif
