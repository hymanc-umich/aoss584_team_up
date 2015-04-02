#include "sensor_thread.h"
#include "board.h"
#include "chprintf.h"


static SerialDriver * DEBUG;

static const I2CConfig ei2cCfg = {
    OPMODE_I2C,
    100000,
    FAST_DUTY_CYCLE_2
};

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
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_56) | 	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) ,
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56)  ,		// ADC SMPR2
    ADC_SQR1_NUM_CH(ANALOG_CHANNELS)  , 	// ADC SQR1
    0,						                // ADC SQR2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)  | 		// ADC SQR3
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN11)
};

static adcsample_t analogSamples[ANALOG_DEPTH*ANALOG_CHANNELS];
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
 * @brief Sensor data collection thread
 */
msg_t sensorThread(void *arg)
{
    sensorThread_t *thread = (sensorThread_t *) arg;
    thread->running = 1;
    msg_t message;
    chMtxObjectInit(&thread->data.mtx); // Initialize Mutex
    
    DEBUG = &DBG_SERIAL;
    // Start I2C
    i2cStart(&II2C_I2CD, &ii2cCfg);
    i2cStart(&EI2C_I2CD, &ei2cCfg);
    
    // Initialize sensors
    tmp275_init(&(thread->tmp275), &EI2C_I2CD, 0b1001000);	            // Initialize TMP275
    si70x0_init(&(thread->extSi7020), &EI2C_I2CD);                      // Initialize External Si7020
    si70x0_init(&(thread->intSi7020), &II2C_I2CD);                      // Initialize Internal Si7020
    ms5607_init(&(thread->ms5607), &EI2C_I2CD, 0b1110111);              // Initialize MS5607
    lsm303_init(&(thread->lsm303), &II2C_I2CD, 0b0011101, 0b0011110);   // Initialize LSM303
    bmp280_init(&(thread->bmp280), &II2C_I2CD, 0b1110110);              // Initialize BMP280
    
    // Run Si70x0 heaters
    si70x0_setHeaterCurrent(&(thread->intSi7020), 14);
    si70x0_heaterEnable(&(thread->intSi7020), true);
    si70x0_setHeaterCurrent(&(thread->extSi7020), 14);
    si70x0_heaterEnable(&(thread->extSi7020), true);
    chThdSleep(1000);
    si70x0_heaterEnable(&(thread->intSi7020), false);
    si70x0_heaterEnable(&(thread->extSi7020), false);
    thread->data.humdInt = 0;
    thread->data.humdExt = 0;

    // Sensor Loop
    systime_t deadline = chVTGetSystemTimeX();
    msg_t i7020stat, e7020stat, tmp275stat, ms5607stat, lsm303stat;
    while(thread->running)
    {
        deadline += MS2ST(100); // Set master sampling rate at ~10Hz
        boardSetLED(1);
    	/*
         * Read I2C sensors
         */

        //External Temp 2
    	tmp275stat = tmp275_readTemperature(&(thread->tmp275), &(thread->data.temp275));
    	if(tmp275stat == MSG_OK)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "EXTEMP:%.1fC\n", thread->data.temp275);
        // TODO: Internal Pressure (BMP280)    

        // Internal Humidity (Si7020)
        i7020stat = si70x0_readHumidity(&(thread->intSi7020), &(thread->data.humdInt));
        float tempi7020;
        i7020stat |= si70x0_readTemperature(&(thread->intSi7020), &tempi7020);
    	//chprintf((BaseSequentialStream *) &DBG_SERIAL, "Si7020 Read: %d\n", (int) ihstat);
        if(i7020stat == MSG_OK)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "IHUM:%.1f%% TEMP:%.2fC\n",thread->data.humdInt, tempi7020);
        
        // External Humidity 1 (Si7020)
        e7020stat = si70x0_readHumidity(&(thread->extSi7020), &(thread->data.humdExt));
        float temp7020ext;
        e7020stat |= si70x0_readTemperature(&(thread->extSi7020), &temp7020ext);
        if(e7020stat == MSG_OK)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "EHUM:%.1f%% ETMP:%.2fC\n",thread->data.humdExt, temp7020ext);

        // External Humidity 2 (HIH6030)


        // External Pressure 2
        ms5607stat = ms5607_readPressureTemperature(&(thread->ms5607), &(thread->data.pressMs5607),&(thread->data.tempMs5607));
        if(ms5607stat == MSG_OK)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "MPRS:%.2fkPa, MTMP:%.2fC\n", thread->data.pressMs5607, thread->data.tempMs5607);
        // Accelerometer
        //lsm303_readAcceleration(&(thread->lsm303), 1);
        // Get out data
        
        // Magnetometer 


    	// Perform ADC read
    	adcConvert(&ADCD1, &analogGrp, analogSamples, ANALOG_DEPTH);   // Convert analog channels
        // Analog processing

        // Sleep
        boardSetLED(0);
        if(chVTGetSystemTimeX() < deadline)
            chThdSleepUntil(deadline);
        else
            deadline = chVTGetSystemTimeX();
    }
    //chThdExit();
    return message;
}

/**
 * 
 */
msg_t sensorThreadStop(sensorThread_t *thread)
{
    thread->running = false;
}
