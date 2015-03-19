/**
 * MS5607-02BA03 Barometric Pressure Sensor Driver for Chibios
 * Author: Cody Hyman
 */

#ifndef _MS5607_H_
#define _MS5607_H_

#include "ch.h"
#include "hal.h"

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
#define MS5607_TEMP_CONVERT	MS5607_D2_CONV_4096

typedef enum
{
    INACTIVE,
    IDLE,
    BUSY
}ms5607_state_t;

typedef struct
{
    ms5607_state_t state;
    I2CDriver *i2c;
    uint8_t addr;
}ms5607_t;

msg_t ms5607_init(ms5607_t *m, I2CDriver *driver, uint8_t baseAddr);
msg_t ms5607_stop(ms5607_t *m, uint8_t stopI2C);
msg_t ms5607_reset(ms5607_t *m);
msg_t ms5607_readPressure(ms5607_t *m, float *pressure);
msg_t ms5607_readTemperature(ms5607_t *m, float *temperature);
msg_t ms5607_readPROMValues(ms5607_t *m);

#endif 