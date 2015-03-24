/**
 * BMP280 I2C Driver for Chibios
 * Author: Cody Hyman
 */

#ifndef _BMP_280_H_
#define _BMP_280_H_

#include "i2c_sensor.h"

typedef struct
{
    I2CSensor_t *i2c;
}bmp280_t;

msg_t bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr);
msg_t bmp280_readPressure(bmp280_t *bmp, float *pressure);

#endif