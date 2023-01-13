#ifndef __APP_LOG_H__
#define __APP_LOG_H__

#include "sys_log.h"

#define APP_LOG_EN

#ifdef APP_LOG_EN
#define APP_LOGE SYS_LOGE
#define APP_LOGW SYS_LOGW
#define APP_LOGI SYS_LOGI
#define APP_LOGD SYS_LOGD
#define APP_LOGV SYS_LOGV
#else
#define APP_LOGE(...) (void)0
#define APP_LOGW(...) (void)0
#define APP_LOGI(...) (void)0
#define APP_LOGD(...) (void)0
#define APP_LOGV(...) (void)0
#endif

#endif
