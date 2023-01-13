#ifndef __SYS_LOG_H__
#define __SYS_LOG_H__

#include <stdio.h>
#include <stdint.h>
#include "sys_io.h"

#define UNUSED(x) (void)(x)

#define SYS_LOG_EN
#define SYS_ASSERT_RESTART 	1

#define ESCAPE          "\033"
#define RED_COLOR       "[0;31m"
#define YELLOW_COLOR    "[0;33m"
#define GREEN_COLOR     "[0;32m"
#define CYAN_COLOR      "[0;36m"
#define RESET_COLOR     "[0m"

#define SYS_PRINTF printf

#ifdef SYS_LOG_EN
#define SYS_LOGE(tag, format, ...) 	SYS_PRINTF(ESCAPE RED_COLOR "[" tag "] " format "\n" ESCAPE RESET_COLOR, ##__VA_ARGS__)
#define SYS_LOGW(tag, format, ...) 	SYS_PRINTF(ESCAPE YELLOW_COLOR "[" tag "] " format "\n" ESCAPE RESET_COLOR, ##__VA_ARGS__)
#define SYS_LOGI(tag, format, ...) 	SYS_PRINTF(ESCAPE GREEN_COLOR "[" tag "] " format "\n" ESCAPE RESET_COLOR, ##__VA_ARGS__)
#define SYS_LOGD(tag, format, ...) 	SYS_PRINTF(ESCAPE CYAN_COLOR "[" tag "] " format "\n" ESCAPE RESET_COLOR, ##__VA_ARGS__)
#define SYS_LOGV(tag, format, ...) 	SYS_PRINTF(ESCAPE RESET_COLOR "[" tag "] " format "\n", ##__VA_ARGS__)
#else
#define SYS_LOGE(...) (void)0
#define SYS_LOGW(...) (void)0
#define SYS_LOGI(...) (void)0
#define SYS_LOGD(...) (void)0
#define SYS_LOGV(...) (void)0
#endif

#define SYS_ASSERT(cond)                                     \
    do                                                       \
    {                                                        \
        if (!(cond))                                         \
        {                                                    \
            SYS_LOGE(ESCAPE RED_COLOR "[ASSERT]", "%s:%d\n" ESCAPE RESET_COLOR, __FILE__, __LINE__); \
            if (SYS_ASSERT_RESTART)                          \
            {                                                \
                sysReset();			                         \
            }                                                \
            do                                               \
            {                                                \
            	toggleLedLife();							 \
                delayMs(200);   							 \
            } while (1);                                     \
        }                                                    \
    } while (0);

#endif
