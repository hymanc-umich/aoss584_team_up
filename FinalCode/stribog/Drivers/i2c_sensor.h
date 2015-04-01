#ifndef _I2C_SENSOR_H_
#define _I2C_SENSOR_H_

#include "ch.h"
#include "hal.h"
#include "../board.h"

#define I2C_SENSOR_SERLOG	&DBG_SERIAL	// Logging serial port
#define I2C_LOGGING TRUE

typedef enum
{
    INACTIVE,
    IDLE,
    BUSY
}I2CSensor_state;

typedef struct
{
    I2CSensor_state state;
    uint8_t address;
    I2CDriver *i2c;
    systime_t timeout;
    char name[11];

    uint8_t *txBuf;
    uint8_t *rxBuf;
}I2CSensor_t;

void I2CSensor_init(I2CSensor_t *sensor, I2CDriver *driver, uint8_t address, uint8_t *txb, uint8_t *rxb, systime_t timeout, char *sensorName);
msg_t I2CSensor_stop(I2CSensor_t *sensor, bool stopI2C);
msg_t I2CSensor_transact(I2CSensor_t *sensor, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc);
msg_t I2CSensor_transact_buf(I2CSensor_t *sensor, uint8_t txc, uint8_t rxc);

#endif