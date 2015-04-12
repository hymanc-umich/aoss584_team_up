#include "ch.h"
#include "hal.h"

#define IWDG_KEY_REFRESH		0xAAAA
#define IWDG_KEY_ACCESS			0x5555
#define IWDG_KEY_WD_START		0xCCCC

#define IWDG_PRESCALER_DIV4		0	//0.125-512ms
#define IWDG_PRESCALER_DIV8 	1 	//0.25-1024ms
#define IWDG_PRESCALER_DIV16	2 	//0.5-2048ms
#define IWDG_PRESCALER_DIV32	3 	//1-4096ms
#define IWDG_PRESCALER_DIV64	4 	//2-8192ms
#define IWDG_PRESCALER_DIV128	5 	//4-16384ms
#define IWDG_PRESCALER_DIV256	6 	//8-32768ms

void watchdog_init(uint8_t prescaler, uint16_t downcount, bool start);
void watchdog_writeKey(uint16_t key);
void watchdog_on(void);
void watchdog_setTimeout(uint16_t reload);
void watchdog_throwBone(void);