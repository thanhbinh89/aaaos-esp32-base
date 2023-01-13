#ifndef SRC_SYS_SYS_IO_H_
#define SRC_SYS_SYS_IO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_LED_LIFE			(GPIO_NUM_32)
#define GPIO_OUTPUT_IO_0		(GPIO_NUM_12)
#define GPIO_OUTPUT_IO_1		(GPIO_NUM_15)
#define GPIO_INPUT_IO_0			(GPIO_NUM_34)
#define GPIO_INPUT_IO_1			(GPIO_NUM_35)

extern void outputInit();
extern void setOutput0(int state);
extern void setOutput1(int state);
extern void inputInit();
/*
 * System core function
 */
extern void platformInit();
extern void sysReset();
extern void ledLifeInit();
extern void setLedLife(bool on);
extern void toggleLedLife();
extern bool getMacAddress(uint8_t *addr);
extern void delayUs(uint32_t us);
extern void delayMs(uint32_t ms);
extern uint32_t millis();
extern uint32_t micros();
#ifdef __cplusplus
}
#endif

#endif /* SRC_SYS_SYS_IO_H_ */
