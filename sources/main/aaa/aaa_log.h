#ifndef __AAA_LOG_H__
#define __AAA_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "sys_log.h"

// #define AAA_LOG_EN

#ifdef AAA_LOG_EN
#define AAA_LOGE SYS_LOGE
#define AAA_LOGW SYS_LOGW
#define AAA_LOGI SYS_LOGI
#define AAA_LOGD SYS_LOGD
#define AAA_LOGV SYS_LOGV
#else
#define AAA_LOGE(...) (void)0
#define AAA_LOGW(...) (void)0
#define AAA_LOGI(...) (void)0
#define AAA_LOGD(...) (void)0
#define AAA_LOGV(...) (void)0
#endif

#ifdef __cplusplus
}
#endif

#endif
