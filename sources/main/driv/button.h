#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define		BUTTON_DRIVER_OK						(0x00)
#define		BUTTON_DRIVER_NG						(0x01)

#define		BUTTON_SHORT_PRESS_MIN_TIME				(100)		/* 100ms */
#define		BUTTON_SHORT_PRESS_MAX_TIME				(1000)		/* 1s */
#define		BUTTON_LONG_PRESS_TIME					(2000)		/* 2s */

#define		BUTTON_DISABLE							(0x00)
#define		BUTTON_ENABLE							(0x01)

/*
 * This define depend on hardware circuit.
 * Note: please change it for respective.
 */
#define		BUTTON_HW_STATE_PRESS					(0x00)
#define		BUTTON_HW_STATE_RELEASE					(0x01)

#define		BUTTON_SW_STATE_RELEASE					(0x00)
#define		BUTTON_SW_STATE_SHORT_HOLD_PRESS		(0x01)
#define		BUTTON_SW_STATE_SHORT_RELEASE_PRESS		(0x02)
#define		BUTTON_SW_STATE_LONG_PRESS				(0x03)

typedef void (*pf_button_ctrl)();
typedef uint8_t (*pf_button_read)(uint8_t);
typedef void (*pf_button_callback)(void*);

typedef struct {
	uint8_t id;
	uint8_t enable;
	uint8_t state;
	uint8_t counter_enable;

	uint32_t counter;
	uint32_t unit;

	pf_button_ctrl init;
	pf_button_read read;

	pf_button_callback callback;
} button_t;

extern uint8_t	button_init(button_t* button, uint32_t u, uint8_t id, pf_button_ctrl init, pf_button_read read, pf_button_callback callback);
extern void	button_enable(button_t* button);
extern void	button_disable(button_t* button);
extern void	button_timer_polling(button_t* button);

#ifdef __cplusplus
}
#endif

#endif //__BUTTON_H__
