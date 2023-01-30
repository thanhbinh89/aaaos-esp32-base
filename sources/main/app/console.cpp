#include "aaa.h"

#include "app.h"
#include "app_log.h"
#include "task_list.h"
#include "task_console.h"

#include "sys_io.h"

#include "esp_err.h"
#include "nvs_flash.h"

/*****************************************************************************/
/*  local declare
 */
/*****************************************************************************/
#define STR_LIST_MAX_SIZE		10
#define STR_BUFFER_SIZE			128

static char cmd_buffer[STR_BUFFER_SIZE];
static char *str_list[STR_LIST_MAX_SIZE];
static uint8_t str_list_len;

static uint8_t str_parser(char *str);
static char* str_parser_get_attr(uint8_t);

/*****************************************************************************/
/*  command function declare
 */
/*****************************************************************************/
static int32_t console_reset(uint8_t *argv);
static int32_t console_ver(uint8_t *argv);
static int32_t console_help(uint8_t *argv);
static int32_t console_reboot(uint8_t *argv);
static int32_t console_fatal(uint8_t *argv);
static int32_t console_boot(uint8_t *argv);
static int32_t console_dbg(uint8_t *argv);
static int32_t console_sensor(uint8_t *argv);
static int32_t console_relay(uint8_t *argv);
static int32_t console_net(uint8_t *argv);
/*****************************************************************************/
/*  command table
 */
/*****************************************************************************/
cmd_line_t lgn_cmd_table[] =
	{

	/*************************************************************************/
	/* system command */
	/*************************************************************************/
		{ (const int8_t*) "reset", console_reset, (const int8_t*) "reset terminal" },
		{ (const int8_t*) "ver", console_ver, (const int8_t*) "version info" },
		{ (const int8_t*) "help", console_help, (const int8_t*) "help command info" },
		{ (const int8_t*) "reboot", console_reboot, (const int8_t*) "reboot system" },
		{ (const int8_t*) "fatal", console_fatal, (const int8_t*) "fatal cmd" },
		{ (const int8_t*) "boot", console_boot, (const int8_t*) "boot cmd" },

	/*************************************************************************/
	/* debug command */
	/*************************************************************************/
		{ (const int8_t*) "dbg", console_dbg, (const int8_t*) "debug cmd" },
		{ (const int8_t*) "sensor", console_sensor, (const int8_t*) "sensor cmd" },
		{ (const int8_t*) "relay", console_relay, (const int8_t*) "relay cmd" },
		{ (const int8_t*) "net", console_net, (const int8_t*) "network cmd" },

	/* End Of Table */
		{ (const int8_t*) 0, (pf_cmd_func) 0, (const int8_t*) 0 } };

uint8_t str_parser(char *str) {
	UNUSED(str);
	strcpy(cmd_buffer, str);
	str_list_len = 0;

	uint8_t i = 0;
	uint8_t str_list_index = 0;
	uint8_t flag_insert_str = 1;

	while (cmd_buffer[i] != 0 && cmd_buffer[i] != '\n' && cmd_buffer[i] != '\r') {
		if (cmd_buffer[i] == ' ') {
			cmd_buffer[i] = 0;
			flag_insert_str = 1;
		}
		else if (flag_insert_str) {
			str_list[str_list_index++] = &cmd_buffer[i];
			flag_insert_str = 0;
		}
		i++;
	}

	cmd_buffer[i] = 0;

	str_list_len = str_list_index;
	return str_list_len;
}

char* str_parser_get_attr(uint8_t index) {
	UNUSED(index);
	if (index < str_list_len) {
		return str_list[index];
	}
	return NULL;
}

/*****************************************************************************/
/*  command function definaion
 */
/*****************************************************************************/
int32_t console_reset(uint8_t *argv) {
	(void) argv;
	SYS_PRINTF("\033[2J\r");
	return 0;
}

int32_t console_ver(uint8_t *argv) {
	(void) argv;
	SYS_PRINTF("-Firmware model:   %s\n", APP_FW_MODEL);
	SYS_PRINTF("-Firmware title    %s\n", APP_FW_TITLE);
	SYS_PRINTF("-Firmware version: %s\n", APP_FW_VERSION);
#ifdef DEBUG
	SYS_PRINTF("-Build type:       DEBUG\n");
#else
	SYS_PRINTF("-Build type:       PROD\n");
#endif
	SYS_PRINTF("-Uptime:           %lu\n", millis());
	SYS_PRINTF("-Heap free:        %lu\n", xPortGetFreeHeapSize());
	return 0;
}

int32_t console_help(uint8_t *argv) {
	uint32_t idx = 0;
	switch (*(argv + 5)) {
	default:
		SYS_PRINTF("\nCOMMANDS INFORMATION:\n\n");
		while (lgn_cmd_table[idx].cmd != (const int8_t*) 0) {
			SYS_PRINTF("%s\n\t%s\n\n", lgn_cmd_table[idx].cmd, lgn_cmd_table[idx].info);
			idx++;
		}
		break;
	}
	return 0;
}

int32_t console_reboot(uint8_t *argv) {
	(void) argv;
	sysReset();
	return 0;
}

int32_t console_fatal(uint8_t *argv) {
	(void) argv;
	return 0;
}

int32_t console_boot(uint8_t *argv) {
	(void) argv;
	return 0;
}

int32_t console_dbg(uint8_t *argv) {
	(void) argv;
	switch (*(argv + 4)) {
	case '1': {
		AAATaskPostMsg(AAA_TASK_CONSOLE_ID, CONSOLE_SEND_TEST, NULL, 0);
	}
	break;

	default:
		break;
	}
	return 0;
}

int32_t console_sensor(uint8_t *argv) {
	(void) argv;
	return 0;
}

int32_t console_relay(uint8_t *argv) {
	(void) argv;	
	return 0;
}

int32_t console_net(uint8_t *argv) {
	(void) argv;
	switch (*(argv + 4)) {
	case 'w': {		
		nvs_handle_t handle;
		esp_err_t err;
		char *ssid, *psk;

		str_parser((char *)argv);
		ssid = str_parser_get_attr(2);
		psk = str_parser_get_attr(3);
		if (strlen(ssid) < 32 || strlen(psk) < 64) {
			SYS_PRINTF("set ssid[%d]: %s, psk[%d]: %s\n", strlen(ssid), ssid, strlen(psk), psk);
		}
		else {
			SYS_PRINTF("string length invalid\n");
			break;
		}

		// Open
		err = nvs_open(NVS_WIFI_INFO, NVS_READWRITE, &handle);
		if (err != ESP_OK) break;
		err = nvs_set_blob(handle, "ssid", ssid, strlen(ssid));
		if (err != ESP_OK) break;
		err = nvs_set_blob(handle, "psk", psk, strlen(psk));
		if (err != ESP_OK) break;
		err = nvs_commit(handle);
		if (err != ESP_OK) break;
		// Close
		nvs_close(handle);

		AAATimerSet(AAA_TASK_NET_ID, NET_START_WIFI, NULL, 0, 1000, false);
	}
	break;

	case 't': {
		nvs_handle_t handle;
		esp_err_t err;
		int8_t type = *(argv + 6) - '0';

		if (type > NET_TYPE_WIFI) break;

		// Open
		err = nvs_open(NVS_NET_CFG, NVS_READWRITE, &handle);
		if (err != ESP_OK) break;
		err = nvs_set_i8(handle, "type", type);
		if (err != ESP_OK) break;
		err = nvs_commit(handle);
		if (err != ESP_OK) break;
		// Close
		nvs_close(handle);

		SYS_PRINTF("set net_type: %d\n", type);
	}
		break;

	case 'i': {
		char *ips;
		if (gotIP(&ips)) {
			SYS_PRINTF("IP: %s\n", ips);
		}
		else {
			SYS_PRINTF("IP: unknown\n");
		}
	}
	break;

	default:
		SYS_PRINTF("net [opt]\nopt:\n");
		SYS_PRINTF(" w ssid psk #set and save wifi info\n");
		SYS_PRINTF(" t type #set net type (0: INT_ETH, 1: EXT_ETH, 2: WIFI)\n");
		SYS_PRINTF(" i #get ip address\n");
		break;
	}
	return 0;
}
