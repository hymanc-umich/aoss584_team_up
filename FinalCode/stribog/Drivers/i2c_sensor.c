#include "i2c_sensor.h"
#include "chprintf.h"

static i2cflags_t errors = 0;
/**
 * @brief Initializer for I2C sensor struct
 * @param sensor I2C sensor struct
 * @param driver I2C Driver
 * @param baseAddress Slave address
 * @param timeout I2C timeout (ms)
 * @param sensorName Sensor print name
 */
void I2CSensor_init(I2CSensor_t *sensor, I2CDriver *driver, uint8_t baseAddress, uint8_t *txb, uint8_t *rxb, systime_t timeout, char *sensorName)
{
    sensor->name[0] = '\0';
    sensor->i2c = driver;
    sensor->address = baseAddress;
    sensor->timeout = timeout;
    sensor->state = IDLE;
    sensor->txBuf = txb;
    sensor->rxBuf = rxb;
    uint8_t i = 0;
    while( (sensorName[i] != '\0') && (i < 10) ) // Copy sensor name
    {
        sensor->name[i] = sensorName[i];
        i++;
    }
    sensor->name[i] = '\0';
    #if I2C_LOGGING
        chprintf((BaseSequentialStream *) I2C_SENSOR_SERLOG, "Starting sensor %s...\n", sensor->name);
    #endif
}

/**
 * @brief Stops I2C sensor
 * @param sensor I2C sensor struct
 * @param stopI2C Stop underlying I2C driver flag
 * @return Status of stopping I2C sensor
 */
msg_t I2CSensor_stop(I2CSensor_t *sensor, bool stopI2C)
{
    msg_t status = 0;
    sensor->state = INACTIVE; // set state to inactive
    #if I2C_LOGGING
        chprintf((BaseSequentialStream *) I2C_SENSOR_SERLOG, "Stopping sensor %s\n", sensor->name);
    #endif
    if(stopI2C) // Stop underlying I2C driver
    {
        i2cStop(sensor->i2c);
    }
    return status;
}

/**
 * @brief Write-Read I2C transaction
 * @param sensor I2C sensor struct
 * @param txb Transmit data buffer
 * @param txc Number of transmit bytes
 * @param rxb Receive data buffer
 * @param rxc Number of expected receive data bytes
 * @return I2C driver status message
 */
msg_t I2CSensor_transact(I2CSensor_t *sensor, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{
    if(sensor->state != INACTIVE)
    {
    	i2cAcquireBus(sensor->i2c);
    	msg_t status = i2cMasterTransmitTimeout(sensor->i2c, sensor->address, txb, txc, rxb, rxc, sensor->timeout);
        i2cReleaseBus(sensor->i2c);
        #if I2C_LOGGING
            if(status != MSG_OK)
            {
                errors = i2cGetErrors(sensor->i2c);
                chprintf((BaseSequentialStream *) I2C_SENSOR_SERLOG, "I2C:%s TRXERR(%d)\n", sensor->name, errors);
            }
            else
            {
                //chprintf((BaseSequentialStream *) I2C_SENSOR_SERLOG, "I2C:%s TRX SUCCESS\n", sensor->name);
            }
        #endif
        return status;
    }
    #if I2C_LOGGING
        chprintf((BaseSequentialStream *) I2C_SENSOR_SERLOG, "I2C:%s INACTIVE\n", sensor->name);
    #endif
    return -99;
}

/**
 * @brief Write-Read I2C transaction from buffers
 * @param sensor I2C sensor struct
 * @param txc Number of transmit bytes in buffer
 * @param rxc Number of expected receive data bytes
 * @return I2C driver status message
 */
msg_t I2CSensor_transact_buf(I2CSensor_t *sensor, uint8_t txc, uint8_t rxc)
{
    return I2CSensor_transact(sensor, sensor->txBuf, txc, sensor->rxBuf, rxc);
}
