#ifndef _SI70X0_H_
#define _SI70X0_H_

#include "ch.h"
#include "hal.h"

#define SI7020_DEFAULT_ADDRESS 0b10000000

// Si7020 Commands
#define SI70X0_MEASURE_RH_HOLD		0xE5
#define SI70X0_MEASURE_RH_NOHOLD	0xF5
#define SI70X0_MEASURE_TEMP_HOLD	0xE3
#define SI70X0_MEASURE_TEMP_NOHOLD	0xF3
#define SI70X0_READ_PREV_TEMP		0xE0
#define SI70X0_RESET			0xFE
#define SI70X0_W_USER_REG1		0xE6
#define SI70X0_R_USER_REG1		0xE7
#define SI70X0_W_HEATER_CTL		0x51
#define SI70X0_R_HEATER_CTL		0x11
#define SI70X0_R_ID_BYTE_1		0xFA0F
#define SI70X0_R_ID_BYTE_2		0xFCC9
#define SI70X0_R_FIRMWARE_VERSION	0x84B8

typedef enum
{
    SI_70X0_INACTIVE,
    SI_70X0_IDLE,
    SI_70X0_READING
}Si70x0_state;

typedef struct
{
    Si70x0_state state;
    uint8_t addr;
    I2CDriver *i2c;
}Si70x0_t;

int8_t si70x0_init(Si70x0_t *s, I2CDriver *driver, uint8_t baseAddr);
int8_t si70x0_stop(Si70x0_t *s, uint8_t stopI2C);
msg_t si70x0_reset(Si70x0_t *s);
msg_t si70x0_setResolution(Si70x0_t *s, uint8_t resolution);
msg_t si70x0_heaterEnable(Si70x0_t *s, uint8_t heaterOn);
msg_t si70x0_setHeaterCurrent(Si70x0_t *s, uint8_t heaterCurrent);
msg_t si70x0_readTemperature(Si70x0_t *s, float *temp);
msg_t si70x0_readHumidity(Si70x0_t *s, uint16_t *humidity);

#endif