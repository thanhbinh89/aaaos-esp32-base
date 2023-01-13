#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

bool getJsonString(char *input, char *output, const char *key) {
	char *begin = NULL, *end = NULL;
	int length = 0;
	char keyString[16] = { 0 };

	strcat(keyString, "\"");
	strcat(keyString, key);
	strcat(keyString, "\"");
	begin = strstr(input, keyString);
	if (begin) {
		begin += strlen(keyString) + 2; /* "key":"value" */
		end = strstr(begin, "\"");
		if (end) {
			length = (uint32_t) end - (uint32_t) begin;
			strncpy(output, begin, length);
			output[length] = 0;
			return true;
		}
	}
	return false;
}

bool getJsonNumber(char *input, uint32_t *output, const char *key, bool lastKey) {
	char *begin = NULL, *end = NULL;
	int length = 0;
	char oBuff[11] = { 0 };
	char keyString[16] = { 0 };

	strcat(keyString, "\"");
	strcat(keyString, key);
	strcat(keyString, "\"");
	begin = strstr(input, keyString);
	if (begin) {
		begin += strlen(keyString) + 1; /* {"key":value,"key":value} */
		if (lastKey) end = strstr(begin, "}");
		else end = strstr(begin, ",");
		if (end) {
			length = (uint32_t) end - (uint32_t) begin;
			strncpy(oBuff, begin, length);
			oBuff[length] = 0;
			*output = atol((const char*) oBuff);
			return true;
		}
	}
	return false;
}

uint8_t sum8Buffer(uint8_t *buff, uint32_t len) {
	uint8_t sum8 = 0;
	uint32_t idx = 0;
	while (idx < len) {
		sum8 += *(buff + idx);
		idx++;
	}
	return sum8;
}
