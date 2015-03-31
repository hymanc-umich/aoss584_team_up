#include "hih6030.h"

/**
 * 
 */
void hih6030_init(hih6030_t *hih, I2CDriver *driver, uint8_t baseAddr)
{
    I2CSensor_init(&(hih->sensor), driver, baseAddr, MS2ST(4));
}

/**
 * 
 */
msg_t hih6030_stop(hih6030_t *hih, bool stopI2C)
{
    return I2CSensor_stop(&(hih->sensor), stopI2C);
}

/**
 * 
 */
msg_t hih6030_readHumidity(hih6030_t *hih, float *humidity)
{
    // TODO
    return 0;
}

/**
 * 
 * 
 */
msg_t hih6030_readTemperature(hih6030_t *hih, float *temp)
{
    // TODO
    return 0;
}