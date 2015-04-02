#include "bmp280.h"

static uint8_t sensCount = 0;

/*
static float bmp280_compensateTemp(int32_t adc)
{
    float v1, v2;
    v1 = (((float) adc)/16384.0f - ((float)dig_T1)/1024.0f) * ((float)dig_T2);
    v2 = ((((float) adc)/131072.0f - ((float) dig_T1)/8192.0f) * 
        (((float)adc)/131072.0f - ((float) dig_T1)/8192.0f)) * ((float) dig_T3);
    return (v1 + v2)/5120.0f;
}
*/

/**
 * @brief Initialize BMP280 I2C pressure sensor
 * @param bmp BMP280 sensor struct
 * @param driver I2C HAL driver
 * @param baseAddr I2C slave base address
 */
void bmp280_init(bmp280_t *bmp, I2CDriver *driver, uint8_t baseAddr)
{
    char name[11] = "BMP280-X";
    name[7] = '0' + sensCount++;
    I2CSensor_init(&bmp->sensor, driver, baseAddr, bmp->txBuffer, bmp->rxBuffer, MS2ST(4), "BMP280");

    msg_t status = I2CSensor_transact(&bmp->sensor, bmp->txBuffer, 2, bmp->rxBuffer, 2);
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
