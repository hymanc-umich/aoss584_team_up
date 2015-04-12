#include "watchdog.h"

/**
 * 
 *
 */
void watchdog_init(uint8_t prescaler, uint16_t downcount, bool start)
{
	watchdog_writeKey(IWDG_KEY_ACCESS);
	IWDG->PR = prescaler;
	watchdog_writeKey(IWDG_KEY_ACCESS);
	watchdog_setTimeout(downcount);
	if(start)
		watchdog_on();
}

/**
 * 
 *
 */
inline void watchdog_writeKey(uint16_t key)
{
	IWDG->KR = key;
}

/**
 *
 *
 */
inline void watchdog_on(void)
{
	watchdog_writeKey(IWDG_KEY_WD_START);
}

/**
 *
 *
 */
inline void watchdog_setTimeout(uint16_t reload)
{
	watchdog_writeKey(IWDG_KEY_ACCESS);
	IWDG->RLR = reload & IWDG_RLR_RL;
}

/**
 * 
 * 
 */
inline void watchdog_throwBone(void)
{
	watchdog_writeKey(IWDG_KEY_REFRESH);
}