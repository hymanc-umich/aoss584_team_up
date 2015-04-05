#ifndef _HIH6030_H_
#define _HIH6030_H_

#include "i2c_sensor.h"

#define HIH6030_SLAVE_ADDRESS 0x27	// Default HIH-6030 slave address

typedef struct
{
    I2CSensor_t sensor;
    uint8_t txBuffer[4];
    uint8_t rxBuffer[5];
    bool measureFlag;
}hih6030_t;

void hih6030_init(hih6030_t *hih, I2CDriver *driver);
msg_t hih6030_stop(hih6030_t *hih, bool stopI2C);
msg_t hih6030_measurementRequest(hih6030_t *hih);
msg_t hih6030_read(hih6030_t *hih, float *humidity, float *temperature);

#endif