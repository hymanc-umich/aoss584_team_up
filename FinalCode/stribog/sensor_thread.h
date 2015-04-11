/**
 * Sensor Data Collection Thread
 * Stribog - Balloon Computer
 * Team Up
 * Author: Cody Hyman
 */

#ifndef _SENSOR_THREAD_H_
#define _SENSOR_THREAD_H_

#include "ch.h"
#include "hal.h"
#include "chmtx.h"

#include "Drivers/bmp280.h"
#include "Drivers/hih6030.h"
#include "Drivers/lsm303.h"
#include "Drivers/ms5607.h"
#include "Drivers/si70x0.h"
#include "Drivers/tmp275.h"
#include "Drivers/rtd.h"
#include "Drivers/mpxm2102.h"

#define ANALOG_DEPTH 32
#define ANALOG_CHANNELS  4

#define ADC_VPERC0 0.00080566406f            // 3.3V/4096
#define VINSNS_RESOLUTION 0.00346435547f

#define ADC_VREF0     1.20f     // 1.2V bandgap reference

typedef struct
{
    mutex_t mtx; // Mutex
    float temp275;          // TMP275 Extenal Temperature
    float tempRtd;          // RTD Analog External Temperature
    float tempMs5607;       // MS5607 External Temperature
    float pressMs5607;      // MS5707 External Pressure
    float tempBmp;          // BMP280 Internal Temperature
    float pressBmp;         // BMP280 Internal Pressure
    float pressMpxm;        // MPXM Analog External Pressure
    float humd7020Int;      // Si7020 Internal Humidity
    float temp7020Int;      // Si7020 Internal Temperature
    float humd7020Ext;      // Si7020 External Humidity
    float temp7020Ext;      // Si7020 External Temperature
    float humd6030;         // HIH-6030 External Humidity
    float temp6030;         // HIH-6030 External Temperature
    float accX;             // LSM303D X-Acceleration
    float accY;             // LSM303D Y-Acceleration
    float accZ;             // LSM303D Z-Acceleration
    float magX;             // LSM303D X-Magnetic Field
    float magY;             // LSM303D Y-Magnetic Field
    float magZ;             // LSM303D Z-Magnetic Field
    float vin;              // Input voltage measurement
    float vref;
}sensorData_t;

// Sensor Thread
typedef struct
{
    uint16_t sleepTime;
    bool running;
    sensorData_t data;
    
    bmp280_t bmp280;
    lsm303_t lsm303;
    ms5607_t ms5607;
    hih6030_t hih6030;
    si70x0_t intSi7020;
    si70x0_t extSi7020;
    tmp275_t tmp275;
}sensorThread_t;


msg_t sensorThread(void *arg);
void sensorThreadStop(sensorThread_t *thread);
void sensorThread_publishDebug(sensorThread_t *thread, BaseSequentialStream *stream);
void sensorThread_publishData(sensorThread_t *thread, BaseSequentialStream *stream);
void sensorThread_dataToCsv(sensorThread_t *thread, char *buffer, uint16_t bufferMax);

#endif