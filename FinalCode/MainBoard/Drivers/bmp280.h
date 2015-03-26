/**
 * BMP280 I2C Driver for ChibiOS
 * Author: Cody Hyman
 */

#ifndef _BMP_280_H_
#define _BMP_280_H_

#include "i2c_sensor.h"

// BMP280 Register Map
#define BMP280_TEMP_XLSB	0xFC
#define BMP280_TEMP_LSB		0xFB
#define BMP280_TEMP_MSB		0xFA
#define BMP280_PRESS_XLSB	0xF9
#define BMP280_PRESS_LSB	0xF8
#define BMP280_PRESS_MSB	0xF7
#define BMP280_CONFIG		0xF5
#define BMP280_CTRL_MEAS	0xF4
#define BMP280_STATUS		0xF3
#define BMP280_RESET		0xE0
#define BMP280_ID		0xD0
#define BMP280_CALIB00		0x88
#define BMP280_CALIB(CPARAM)	(0x88+2*N)	

/**
 * BMP280 Pressure Sensor struct
 */
typedef struct
{
    I2CSensor_t sensor;
    float lastTemp;
    float lastPressure;
    int32_t T[3];    // Temperature compensation coefficients
    int32_t P[9];    // Pressure compensation coefficients
}bmp280_t;

void bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr);
msg_t bmp280_stop(bmp280_t *bmp, bool stopI2C);
msg_t bmp280_reset(bmp280_t *bmp);
msg_t bmp280_readPressure(bmp280_t *bmp, float *pressure);
msg_t bmp280_readTemperature(bmp280_t *bmp, float *temp);
#endif