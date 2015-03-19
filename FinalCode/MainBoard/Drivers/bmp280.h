/**
 * BMP280 I2C Driver for Chibios
 * Author: Cody Hyman
 */

#ifndef _BMP_280_H_
#define _BMP_280_H_

#include "ch.h"
#include "hal.h"

typedef enum
{
    
}bmp280_state_t;

typedef struct
{
    I2CDriver *i2c;
    uint8_t addr;
    bmp280_state_t state;
}bmp280_t;

msg_t bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr);
msg_t bmp280_readPressure(bmp280_t *bmp, float *pressure);

#endif