#include "sensor_thread.h"
#include "board.h"
#include "chprintf.h"

#define STARTUP_VALUE -99.0f
// External I2C configuration
static const I2CConfig ei2cCfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2
};

// Internal I2C configuration
static const I2CConfig ii2cCfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2
};

/*
 * ADC1.0: VINSNS
 * ADC1.10 MPXM Pressure Sensor
 * ADC1.11 Temperature Bridge
 */
static const ADCConversionGroup analogGrp =
{
    FALSE,
    ANALOG_CHANNELS,
    NULL,
    NULL,
    0,						                // ADC CR1
    ADC_CR2_SWSTART,				        // ADC CR2
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_144) | 	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_144) ,
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_144)  ,	// ADC SMPR2
    ADC_SQR1_NUM_CH(ANALOG_CHANNELS)  , 	// ADC SQR1
    0,						                // ADC SQR2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)  | 		// ADC SQR3 (VINSNS, Pressure, Temperature)
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN11)
};

// ADC data storage
static adcsample_t analogSamples[2][ANALOG_DEPTH * ANALOG_CHANNELS];
static uint8_t activeAnalogCh = 0;

/*
 * {
    mutex_t mtx; // Mutex
    float tempBmp;
    float temp275;
    float tempRtd;
    float pressBmp;
    float pressMpxm;
    float pressMs5607;
    float humdInt, humdExt, humd6030;
    float accX, accY, accZ;
    float magX, magY, magZ;
}sensorData_t;
*/

/**
 * @brief Takes mean of ADC sequence data
 * @param buffer Data buffer start
 * @param First element in sequence buffer offset
 * @param nChannels Number of channels stored in array
 * @param bufferLength Number of reads per channel (buffer depth)
 * @return Mean FP value of ADC channel over buffer
 */
float adcMeanFloat(uint16_t *buffer, uint16_t bufferOffset, uint16_t nChannels, uint16_t bufferDepth)
{
    uint16_t i;
    float count = 0.0f;
    float analogSum = 0;
    for(i = bufferOffset; i < (nChannels * bufferDepth); i = i+nChannels)
    {
        count = count + 1.0f;
        analogSum += (float) buffer[i];
    }
    float out = ((float) analogSum)/count;
    return out;
}

/**
 * @brief Compute mean measured voltage of a given ADC sequence channel
 * @param buffer Data buffer start
 * @param First element in sequence buffer offset
 * @param nChannels Number of channels stored in array
 * @param bufferLength Number of reads per channel (buffer depth)
 * @return Mean FP voltage over buffer length
 */
inline float adcMeanV(uint16_t *buffer, uint16_t bufferOffset, uint16_t nChannels, uint16_t bufferDepth)
{
    return adcMeanFloat(buffer, bufferOffset, nChannels, bufferDepth)*ADC_VPERC;
}

/**
 * @brief Compute integer mean of ADC values for a given channel
 * @param buffer Data buffer start
 * @param First element in sequence buffer offset
 * @param nChannels Number of channels stored in array
 * @param bufferLength Number of reads per channel (buffer depth)
 * @return Mean integer value of ADC channel over buffer
 */
uint16_t adcMean(uint16_t *buffer, uint16_t bufferOffset, uint16_t nChannels, uint16_t bufferDepth)
{
    uint16_t i;
    uint16_t count = 0;
    uint32_t analogSum = 0;
    for(i = bufferOffset; i<(nChannels * bufferDepth); i = i+nChannels)
    {
        count++;
        analogSum += buffer[i];
    }
    return (uint16_t) (analogSum/count);
}

/**
 *
 *
 */
void sensorThread_publishDebug(sensorThread_t *thread, BaseSequentialStream *stream)
{
    chprintf(stream, 
    "BMPTEMP:%.2fC    BMPPRESS:%.2fkPa\nEXTEMP:%.1fC    \nEHUM:%.1f%%    EHTEMP:%.2fC\nIHUM:%.1f%%    \
    IHTEMP:%.2fC\nMPRS:%.2fkPa    MTEMP:%.2fC\nHIHHUM:%.2f%%    \
    HIHTEMP:%.2fC\nVIN:%.2fV\nAPRESS:%.3fkPa\nATEMP:%.3fC\n",
    thread->data.tempBmp, thread->data.pressBmp,
    thread->data.temp275,
    thread->data.humd7020Ext, thread->data.temp7020Ext,
    thread->data.humd7020Int, thread->data.temp7020Int,
    thread->data.pressMs5607, thread->data.tempMs5607,
    thread->data.humd6030, thread->data.temp6030,
    thread->data.vin, thread->data.pressMpxm, thread->data.tempRtd);
}

/**
 *
 *
 */
void sensorThread_publishData(sensorThread_t *thread, BaseSequentialStream *stream)
{
    // Print to XBee
    chprintf(stream, 
        "<DATA>ET:%.1f,EH:%.1f,EHT:%.2f,IH:%.1f,IHT:%.2f,MP:%.2f,MT:%.2f,HH:%.2f,HT:%.2f,V:%.2f,AP:%.3f,AT:%.3f</DATA>\n",
        thread->data.temp275,
        thread->data.humd7020Ext, thread->data.temp7020Ext,
        thread->data.humd7020Int, thread->data.temp7020Int,
        thread->data.pressMs5607, thread->data.tempMs5607,
        thread->data.humd6030, thread->data.temp6030,
        thread->data.vin, thread->data.pressMpxm, thread->data.tempRtd);
}

/**
 *
 *
 */
void sensorThread_dataToCsv(sensorThread_t *thread, char *buffer, uint16_t bufferMax)
{
    chsnprintf(buffer, bufferMax, "%.1f,%.1f,%.2f,%.1f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f", 
        thread->data.temp275,
        thread->data.humd7020Ext, thread->data.temp7020Ext,
        thread->data.humd7020Int, thread->data.temp7020Int,
        thread->data.pressMs5607, thread->data.tempMs5607,
        thread->data.humd6030, thread->data.temp6030,
        thread->data.vin, thread->data.pressMpxm, thread->data.tempRtd);
}

/**
 * @brief Sensor data collection thread
 * @param arg Thread argument (sensorThread struct)
 * @return Thread return message
 */
msg_t sensorThread(void *arg)
{
    sensorThread_t *thread = (sensorThread_t *) arg;
    thread->running = 1;
    msg_t message;
    chMtxObjectInit(&thread->data.mtx); // Initialize Mutex
    
    // Start I2C
    i2cStart(&II2C_I2CD, &ii2cCfg);
    i2cStart(&EI2C_I2CD, &ei2cCfg);
    
    // Initialize sensors
    tmp275_init(&(thread->tmp275), &EI2C_I2CD, 0b1001000);	            // Initialize TMP275
    si70x0_init(&(thread->extSi7020), &EI2C_I2CD);                      // Initialize External Si7020
    si70x0_init(&(thread->intSi7020), &II2C_I2CD);                      // Initialize Internal Si7020
    ms5607_init(&(thread->ms5607), &EI2C_I2CD, 0b1110111);              // Initialize MS5607
    lsm303_init(&(thread->lsm303), &II2C_I2CD);   // Initialize LSM303
    bmp280_init(&(thread->bmp280), &II2C_I2CD, 0b1110110);              // Initialize BMP280
    hih6030_init(&(thread->hih6030), &EI2C_I2CD);
    // Run Si70x0 heaters
    si70x0_setHeaterCurrent(&(thread->intSi7020), 14);
    si70x0_heaterEnable(&(thread->intSi7020), true);
    si70x0_setHeaterCurrent(&(thread->extSi7020), 14);
    si70x0_heaterEnable(&(thread->extSi7020), true);
    chThdSleep(1000);
    si70x0_heaterEnable(&(thread->intSi7020), false);
    si70x0_heaterEnable(&(thread->extSi7020), false);

    thread->data.temp275 = STARTUP_VALUE;
    thread->data.tempRtd = STARTUP_VALUE;
    thread->data.tempMs5607 = STARTUP_VALUE;
    thread->data.pressMs5607 = STARTUP_VALUE;
    thread->data.tempBmp = STARTUP_VALUE;
    thread->data.pressBmp = STARTUP_VALUE;
    thread->data.pressMpxm = STARTUP_VALUE;
    thread->data.humd7020Int = STARTUP_VALUE;
    thread->data.temp7020Int = STARTUP_VALUE;
    thread->data.humd7020Ext = STARTUP_VALUE;
    thread->data.temp7020Ext = STARTUP_VALUE;
    thread->data.humd6030 = STARTUP_VALUE;
    thread->data.temp6030 = STARTUP_VALUE;
    thread->data.accX = STARTUP_VALUE;
    thread->data.accY = STARTUP_VALUE;
    thread->data.accZ = STARTUP_VALUE;
    thread->data.magX = STARTUP_VALUE;
    thread->data.magY = STARTUP_VALUE;
    thread->data.magZ = STARTUP_VALUE;
    thread->data.vin = STARTUP_VALUE;

    /*
     * ===== Sensor Loop =====
     */
    systime_t deadline = chVTGetSystemTimeX();
    msg_t i7020stat, e7020stat, tmp275stat, ms5607stat, lsm303stat;
    bool publishFlag = FALSE;
    while(thread->running)
    {
        deadline += MS2ST(500); // Set master sampling rate at ~10Hz
        boardSetLED(1);

        // Start HIH6030 measurement request
        hih6030_measurementRequest(&(thread->hih6030));

        // Perform ADC read
        adcConvert(&ADCD1, &analogGrp, analogSamples[activeAnalogCh ^ 1], ANALOG_DEPTH);   // Convert analog channels

    	/*
         * ===== Read I2C sensors =====
         */

        //External Temp 2
    	tmp275stat = tmp275_readTemperature(&(thread->tmp275), &(thread->data.temp275));

        // TODO: Internal Pressure (BMP280)    
        bmp280_readTemperature(&(thread->bmp280), &(thread->data.tempBmp));
        bmp280_readPressure(&(thread->bmp280), &(thread->data.pressBmp));

        // Internal Humidity (Si7020)
        i7020stat = si70x0_readHumidity(&(thread->intSi7020), &(thread->data.humd7020Int));
        i7020stat |= si70x0_readTemperature(&(thread->intSi7020), &(thread->data.temp7020Int));
    	//chprintf((BaseSequentialStream *) &DBG_SERIAL, "Si7020 Read: %d\n", (int) ihstat);

        // External Humidity 1 (Si7020)
        e7020stat = si70x0_readHumidity(&(thread->extSi7020), &(thread->data.humd7020Ext));
        e7020stat |= si70x0_readTemperature(&(thread->extSi7020), &(thread->data.temp7020Ext));

        // External Humidity 2 (HIH6030)
        hih6030_read(&(thread->hih6030), &(thread->data.humd6030), &(thread->data.temp6030));

        // External Pressure 2
        ms5607stat = ms5607_readPressureTemperature(&(thread->ms5607), &(thread->data.pressMs5607),&(thread->data.tempMs5607));
        
        /*if(ms5607stat == MSG_OK)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "MPRS:%.2fkPa, MTMP:%.2fC\n", thread->data.pressMs5607, thread->data.tempMs5607);*/
        // Accelerometer
        //lsm303_readAcceleration(&(thread->lsm303), 1);
        //lsm303_readMagnetometer(&(thread->lsm303), 1);
        // Get out data
        
        // Magnetometer 


        /* 
         * ==== Analog processing =====
         */ 
        // Input voltage
        thread->data.vin = adcMeanFloat(analogSamples[activeAnalogCh], 0, ANALOG_CHANNELS, ANALOG_DEPTH) * VINSNS_RESOLUTION - 0.06;

        // Analog pressure
        thread->data.pressMpxm = mpxmVToPressure(adcMeanV(analogSamples[activeAnalogCh], 1, ANALOG_CHANNELS, ANALOG_DEPTH));

        // Analog Temperature
        //thread->data.tempRtd = RTD_vToTemp(adcMeanV(analogSamples[activeAnalogCh], 2, ANALOG_CHANNELS, ANALOG_DEPTH));
        thread->data.tempRtd = RTD_vToTemp(ADC_VPERC*analogSamples[activeAnalogCh][2]);
        //thread->data.tempRtd = analogSamples[activeAnalogCh][2]*1.0f; // ADC output test

        activeAnalogCh ^= 1; // Toggle analog buffer

        /* ===== Printf Dump ===== */
        sensorThread_publishDebug(thread, (BaseSequentialStream *) &DBG_SERIAL);
        if(publishFlag)
        {
            sensorThread_publishData(thread, (BaseSequentialStream *) &COM_SERIAL);
        }
        publishFlag = !publishFlag;

        /* ===== Sleep ==== */
        boardSetLED(0);
        if(chVTGetSystemTimeX() < deadline)
            chThdSleepUntil(deadline);
        else
        {
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "SENSOR DEADLINE EXCEEDED\n");
            deadline = chVTGetSystemTimeX();
        }
    }
    return message;
}

/**
 * 
 */
void sensorThreadStop(sensorThread_t *thread)
{
    thread->running = false;
}
