/**
 * MS5607-02BA03 Barometric Pressure Sensor Driver for Chibios
 * Author: Cody Hyman
 */

#ifndef _MS5607_H_
#define _MS5607_H_

#include "i2c_sensor.h"

#define MS5607_RESET		0x1E
#define MS5607_D1_CONV_256	0x40
#define MS5607_D1_CONV_512	0x42
#define MS5607_D1_CONV_1024	0x44
#define MS5607_D1_CONV_2048	0x46
#define MS5607_D1_CONV_4096	0x48
#define MS5607_D2_CONV_256	0x50
#define MS5607_D2_CONV_512	0x52
#define MS5607_D2_CONV_1024	0x54
#define MS5607_D2_CONV_2048	0x56
#define MS5607_D2_CONV_4096	0x58
#define MS5607_READ_ADC_RESULT	0x00
#define MS5607_READ_PROM(ADDR)  (0xA0 + ADDR)


#define MS5607_PRESSURE_CONVERT	MS5607_D1_CONV_4096
#define MS5607_TEMPERATURE_CONVERT	MS5607_D2_CONV_4096

typedef struct
{
    I2CSensor_t sensor;	// I2C sensor struct
    uint16_t cal[6];	// Calibration coefficients
    float lastTemp;
    float lastPress;
    uint8_t txBuffer[4];
    uint8_t rxBuffer[16];
}ms5607_t;

msg_t ms5607_init(ms5607_t *m, I2CDriver *driver, uint8_t baseAddr);
msg_t ms5607_stop(ms5607_t *m, bool stopI2C);
msg_t ms5607_reset(ms5607_t *m);
msg_t ms5607_readPressureTemperature(ms5607_t *m, float *pressure, float *temperature);

#endif 