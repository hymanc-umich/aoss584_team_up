#ifndef _SI70X0_H_
#define _SI70X0_H_

#include "ch.h"
#include "hal.h"

typedef enum
{
    SI_70X0_INACTIVE,
    SI_70X0_IDLE,
    SI_70X0_READING
}Si70x0_state;

typedef struct
{
    Si70x0_state state;
    uint8_t baseAddr;
    I2CDriver *i2c;
}Si70x0_t;

int8_t si70x0_init(Si70x0_t *s, I2CDriver *driver, uint8_t baseAddr);
int8_t si70x0_stop(Si70x0_t *s, uint8_t stopI2C);
int8_t si70x0_setResolution(Si70x0_t *s, uint8_t resolution);
int8_t si70x0_heaterEnable(Si70x0_t *s, uint8_t heaterOn);
int8_t si70x0_setHeaterCurrent(Si70x0_t *s, uint8_t heaterCurrent);
int8_t si70x0_readTemperature(Si70x0_t *s, int16_t *temp);
int8_t si70x0_readHumidity(Si70x0_t *s, uint16_t *humdity);

#endif