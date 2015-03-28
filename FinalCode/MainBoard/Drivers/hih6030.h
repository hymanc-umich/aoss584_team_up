#ifndef _HIH6030_H_
#define _HIH6030_H_

#include "i2c_sensor.h"

typedef struct
{
    I2CSensor_t sensor;
}hih6030_t;

void hih6030_init(hih6030_t *hih, I2CDriver *driver, uint8_t baseAddr);
msg_t hih6030_stop(hih6030_t *hih, bool stopI2C);
msg_t hih6030_readHumidity(hih6030_t *hih, float *humidity);
msg_t hih6030_readTemperature(hih6030_t *hih, float *temp);

#endif