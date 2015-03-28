#include "sensor_thread.h"

static const I2CConfig i2cfg = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
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
    0,						// ADC CR1
    ADC_CR2_SWSTART,				// ADC CR2
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_56) | 	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) ,
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56)  ,		// ADC SMPR2
    ADC_SQR1_NUM_CH(ANALOG_CHANNELS), 		// ADC SQR1
    0,						// ADC SQR2
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
    msg_t message;
    chMtxObjectInit(&thread->data.mtx); // Initialize Mutex
    
    // Start I2C
    i2cStart(&II2C_I2CD, &i2cfg);
    i2cStart(&EI2C_I2CD, &i2cfg);
    
    // Initialize sensors
    tmp275_init(&(thread->tmp275), &EI2C_I2CD, 0b1001000);	// Initialize TMP275
    si70x0_init(&(thread->extSi7020), &EI2C_I2CD, 0b1000000); // Initialize External Si7020
    si70x0_init(&(thread->intSi7020), &II2C_I2CD, 0b1000000); // Initialize 
    ms5607_init(&(thread->ms5607), &EI2C_I2CD, 0b1110110);
    
    // Sensor Loop
    while(thread->running)
    {
	// Read I2C sensors
	tmp275_readTemperature(&(thread->tmp275), &(thread->data.temp275));
	si70x0_readHumidity(&(thread->intSi7020), &(thread->data.humdInt));
	ms5607_readTemperature(&(thread->ms5607), &(thread->data.tempMs5607));
	ms5607_readPressure(&(thread->ms5607), &(thread->data.pressMs5607));
	// Perform ADC read
	adcConvert(&ADCD1, &analogGrp, analogSamples, ANALOG_DEPTH);
   
	chThdSleepMilliseconds(100); // Read ~10Hz
    }

    return message;
}

/**
 * 
 */
msg_t sensorThreadStop(sensorThread_t *thread)
{
    thread->running = false;
}
