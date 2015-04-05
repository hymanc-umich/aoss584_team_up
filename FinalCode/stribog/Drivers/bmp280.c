#include "bmp280.h"
#include "board.h"
#include "chprintf.h"

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

    uint8_t i;
    msg_t status;
    for(i=0; i<3; i++)
    {
        bmp->txBuffer[0] = BMP280_CALIB(i); 
        status = I2CSensor_transact_buf(&bmp->sensor, 1, 2);
        if(status == MSG_OK)
            bmp->T[i] = (bmp->rxBuffer[1]<<8) + bmp->rxBuffer[0];
        else
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "BMP280: ERROR READING T-PARAMETER");
    }
    for(i=0; i<9; i++)
    {
        bmp->txBuffer[0] = BMP280_CALIB(3+i);
        status = I2CSensor_transact_buf(&bmp->sensor, 1, 2);
        if(status == MSG_OK)
            bmp->P[i] = (bmp->rxBuffer[1]<<8) + bmp->rxBuffer[0];
        else
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "BMP280: ERROR READING P-PARAMETER");
    }
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
    bmp->txBuffer[0] = BMP280_RESET;
    return I2CSensor_transact_buf(&bmp->sensor, 1, 0);
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
    bmp->txBuffer[0] = BMP280_PRESS_MSB;
    status = I2CSensor_transact_buf(&(bmp->sensor),1,3);
    if(status == MSG_OK)
    {
        int32_t rawPressure = (bmp->rxBuffer[0]<<12) + (bmp->rxBuffer[1]<<4)+ (bmp->rxBuffer[2]>>4);
	   // TODO: compensation
        float compPress;
        if(pressure != NULL)
        {
            *pressure = compPress;
        }
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
    bmp->txBuffer[0] = BMP280_TEMP_MSB;
    status = I2CSensor_transact_buf(&(bmp->sensor),1,3);
    if(status == MSG_OK)
    {
        int32_t rawTemp = (bmp->rxBuffer[0]<<12) + (bmp->rxBuffer[1]<<4)+ (bmp->rxBuffer[2]>>4);
	    // TODO: compensation
        float compTemp = 0;
        if(temp != NULL)
        {
            *temp = compTemp;
        }
    }
    return status;
}
