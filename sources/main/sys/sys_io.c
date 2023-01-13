#include "esp_err.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "driver/gpio.h"

#include "aaa.h"
#include "sys_io.h"

#define GET_BIT(byte, index)	((byte >> index) & 0x01)

void outputInit() {
	//zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1));
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void setOutput0(int state) {
	gpio_set_level(GPIO_OUTPUT_IO_0, state);
}

void setOutput1(int state) {
	gpio_set_level(GPIO_OUTPUT_IO_1, state);
}

void inputInit() {
	//zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1));
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    gpio_config(&io_conf);
}

/*
 * System core function
 */
void platformInit() {
	//Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      nvs_flash_erase();
      nvs_flash_init();
    }

	ledLifeInit();
}

void sysReset() {
	esp_restart();
}

void ledLifeInit() {
	//zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ull<<GPIO_LED_LIFE);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void setLedLife(bool on) {
	gpio_set_level(GPIO_LED_LIFE, on ? 1 : 0);
}

void toggleLedLife() {
	static bool on = false;
	on = !on;
	setLedLife(on);
}

bool getMacAddress(uint8_t *addr) {
	uint8_t ret = esp_efuse_mac_get_default(addr);
    return (ret == ESP_OK);
}

void delayUs(uint32_t us) {
	int64_t next = esp_timer_get_time() + us;
	while (esp_timer_get_time() < next) {
		;
	}
}

void delayMs(uint32_t ms) {
	for (uint32_t c = 0; c < ms; c++) {
		delayUs(1000);
	}
}

uint32_t millis() {
	return (uint32_t)(esp_timer_get_time()/1000);
}

uint32_t micros() {
	return (uint32_t)(esp_timer_get_time());
}
