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
#define BMP280_ID			0xD0
#define BMP280_CALIB00		0x88
#define BMP280_CALIB(CPARAM)	(0x88+2*CPARAM)	

// BMP280 Configuration Register Definitions
#define BMP280_FILTER_OFF	(0b000<<2)
#define BMP280_FILTER_2		(0b001<<2)
#define BMP280_FILTER_4		(0b010<<2)
#define BMP280_FILTER_8		(0b011<<2)
#define BMP280_FILTER_16	(0b100<<2)

#define BMP280_T_SB_0M5		(0b000<<5)
#define BMP280_T_SB_62M5	(0b001<<5)
#define BMP280_T_SB_125M	(0b010<<5)
#define BMP280_T_SB_250M	(0b011<<5)
#define BMP280_T_SB_500M	(0b100<<5)
#define BMP280_T_SB_1000M	(0b101<<5)
#define BMP280_T_SB_2000M	(0b110<<5)
#define BMP280_T_SB_4000M	(0b111<<5)

// BMP280 Measurement Control Register Definitions
#define BMP280_MODE_SLEEP  0b00
#define BMP280_MODE_FORCED 0b01
#define BMP280_MODE_NORMAL 0b11

#define BMP280_OSRS_T_SKIP	(0b000<<5)
#define BMP280_OSRS_T_X1	(0b001<<5)
#define BMP280_OSRS_T_X2	(0b010<<5)
#define BMP280_OSRS_T_X4	(0b011<<5)
#define BMP280_OSRS_T_X8	(0b100<<5)
#define BMP280_OSRS_T_X16	(0b101<<5)

#define BMP280_OSRS_P_SKIP	(0b000<<2)
#define BMP280_OSRS_P_X1	(0b001<<2)
#define BMP280_OSRS_P_X2	(0b010<<2)
#define BMP280_OSRS_P_X4	(0b011<<2)
#define BMP280_OSRS_P_X8	(0b100<<2)
#define BMP280_OSRS_P_X16	(0b101<<2)
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
    uint8_t txBuffer[4];
    uint8_t rxBuffer[16];
}bmp280_t;

void bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr);
msg_t bmp280_stop(bmp280_t *bmp, bool stopI2C);
msg_t bmp280_reset(bmp280_t *bmp);
msg_t bmp280_readPressure(bmp280_t *bmp, float *pressure);
msg_t bmp280_readTemperature(bmp280_t *bmp, float *temp);
#endif