#include "tmp275.h"

/**
 * @brief Initialize TMP275 temperature sensor
 * @param tmp TMP275 sensor struct
 * @param driver I2C HAL driver
 * @param baseAddr Base I2C slave address
 */
void tmp275_init(tmp275_t *tmp, I2CDriver *driver, uint8_t baseAddr)
{
    I2CSensor_init(&tmp->sensor, driver, baseAddr, MS2ST(4));
    // TODO: set config to defaults
}

/**
 * @brief Stops TMP275 sensor
 * @param tmp TMP275 sensor struct
 * @param stopI2C Stop underlying I2C driver
 * @return I2C driver status message
 */
msg_t tmp275_stop(tmp275_t *tmp, bool stopI2C)
{
    return I2CSesnsor_stop(&tmp->sensor, stopI2C);
}

/**
 * @brief Write to the configuration register
 * @param tmp TMP275 sensor struct
 * @param value Register value
 * @return I2C driver status message
 */
msg_t tmp275_writeConfig(tmp275_t *tmp, uint8_t value)
{
    uint8_t txBytes[2] = {TMP275_CONFIGURATION, value}
    I2CSensor_transact(&tmp->sensor, txBytes, 2, NULL, 0);
}

/**
 * @brief Read formatted temperature
 * @param tmp TMP275 sensor struct
 * @param temp Temperature return value
 * @return I2C driver status
 */
msg_t tmp275_readTemperature(tmp275_t *tmp, float *temp)
{
    uint8_t tregAddr = TMP275_TEMPERATURE;
    uint8_t tbuf[2];
    msg_t status = I2CSensor_transact(&tmp->sensor, &tregAddr, 1, tbuf, 2);
    if(status == 0)
    {
	int16_t rawTemp = (int16_t) ((tbuf[0]<<8)|(tbuf[1]));
	*temp = (1.0f*rawTemp)*TMP275_RESOLUTION; // Format temperature
    }
    return status;
    
}

/**
 * @brief Writes the temperature high threshold register
 * @param tmp TMP275 sensor struct
 * @param tempLimit Temperature ADC alert limit
 * @return I2C driver status
 */
msg_t tmp275_writeTHigh(tmp275_t *tmp, int16_t tempLimit)
{
    uint8_t txBytes[2] = {(uint8_t)(tempLimit>>8)&0xFF, (uint8_t)(tempLimit&0xFF)};
    return I2CSensor_transact(&tmp->sensor, txBytes, 2, NULL, 0);
}

/**
 * @brief Writes the temperature low threshold register
 * @param tmp TMP275 sensor struct
 * @param tempLimit Temperature ADC alert limit
 * @return I2C driver status
 */
msg_t tmp275_writeTLow(tmp275_t *tmp, int16_t tempLimit)
{
    uint8_t txBytes[2] = {(uint8_t) (tempLimit>>8)&0xFF, (uint8_t)(tempLimit&0xFF)};
    return I2CSensor_transact(&tmp->sensor, txBytes, 2, NULL, 0);
}