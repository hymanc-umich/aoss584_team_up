#ifndef _TMP275_H_
#define _TMP275_H_

#include "i2c_sensor.h"

typedef struct
{
    I2CSensor_t sensor;
}tmp275_t;

tmp275_init(tmp275_t *tmp, I2CDriver *driver, uint8_t baseAddr);

#endif