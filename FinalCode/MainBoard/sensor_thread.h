#ifndef _SENSOR_THREAD_H_
#define _SENSOR_THREAD_H_

#include "ch.h"
#include "hal.h"

#include "Drivers/bmp280.h"
#include "Drivers/lsm303.h"
#include "Drivers/ms5607.h"
#include "Drivers/si70x0.h"
#include "Drivers/tmp275.h"
#include "Drivers/rtd.h"

typedef struct
{
    // Mutex
    
    
}sensorData_t;

// Sensor Thread
typedef struct
{
    uint16_t sleepTime;
    bool running;
    sensorData_t *data;
    I2CDriver *i2c;
    ADCDriver *adc;
    
    bmp280_t bmp280;
    lsm303_t lsm303;
    ms5607_t ms5607;
    si70x0_t si7020;
    rtd_t rtd;
    tmp275_t tmp275;
}sensorThread_t;


msg_t sensorThread(void *args);
msg_t sensorThreadStop(sensorThread_t);
sensorData_t *getSensorData(sensorThread_t *thread);
#endif