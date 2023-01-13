#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

extern bool getJsonString(char *input, char *output, const char *key);
extern bool getJsonNumber(char *input, uint32_t *output, const char *key, bool lastKey);
extern uint8_t sum8Buffer(uint8_t *buff, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
