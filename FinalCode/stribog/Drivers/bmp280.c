#include "bmp280.h"

/**
 * @brief Initialize BMP280 I2C pressure sensor
 * @param bmp BMP280 sensor struct
 * @param driver I2C HAL driver
 * @param baseAddr I2C slave base address
 */
void bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr)
{
    I2CSensor_init(&bmp->sensor, driver, baseAddr, MS2ST(4));
    // TODO: Read cal coefficients
}

/**
 * @brief Stop BMP280 sensor
 * @param bmp BMP280 sensor struct
 * @param stopI2C Stop underlying I2C driver
 */
msg_t bmp280_stop(bmp280_t *bmp, bool stopI2C)
{
    return I2CSensor_stop(&bmp->sensor, stopI2C);
}

/**
 * @brief Bus sensor reset
 * @param bmp BMP280 sensor struct
 * @return I2C driver status
 */
msg_t bmp280_reset(bmp280_t *bmp)
{
    uint8_t regId = BMP280_RESET;
    return I2CSensor_transact(&bmp->sensor, &regId, 1, NULL, 0);
}

/**
 * @brief Performs a compensated pressure read
 * @param bmp BMP280 sensor struct
 * @param pressure Pressure return address
 * @return I2C sensor status
 */
msg_t bmp280_readPressure(bmp280_t *bmp, float *pressure)
{
    msg_t status;
    uint8_t pressData[3];
    // TODO: read and align pressure registers
    if(status == 0)
    {
	// TODO: compensation
    }
    return status;
}

/**
 * @brief Performs a compensated temperature read 
 * @param bmp BMP280 sensor struct
 * @param temp Temperature return address
 * @return I2C sensor status
 */
msg_t bmp280_readTemperature(bmp280_t *bmp, float *temp)
{
    msg_t status;
    uint8_t tempData[3];
    // TODO
    if(status == 0)
    {
	// TODO: compensation
    }
    return status;
}
