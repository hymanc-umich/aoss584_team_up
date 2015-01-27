#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include <chstreams.h>
#include <string.h>

#include "sdmmcsimple.h"
#include "datalogger.h"

#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/* ADC Defines */
#define ADC_BUF_DEPTH 1 // Double buffered ADC
#define SENSOR_ADC_CH_NUM 4
#define ACC_ADC_CH_NUM 3

// TODO: Time values

/* Accelerometer Measurement */
typedef struct
{
    uint16_t x;		// X-axis
    uint16_t y;		// Y-axis
    uint16_t z;		// Z-axis
}accMeasurement_t;

/* Analog Sensor Measurement */
typedef struct
{
    uint16_t temp[2];	// Raw Temperature
    uint16_t humd;	// Raw humidity
    uint16_t press;	// Raw pressure
}sensorMeasurement_t;

/* GPS Location */
typedef struct
{
    int32_t lat;	// Latitude (x10^6)
    int32_t lon;	// Longitude (x10^6)
    int32_t alt;	// Altitude
    uint8_t sat; 	// Satellite count
}gpsLocation_t;

static sdmmc_t sd;
static datalogger_t logger;

static logfile_t sensorLog;

void initialize();

static sensorMeasurement_t SENSORS;

MMCDriver MMCD1;

/**
 * @brief Accelerometer ADC callback
 */
static void accCallback(ADCDriver *adcd, adcsample_t *buf, size_t n)
{
    // TODO: Format and store accelerometer data
}

/**
 * @brief Analog sensor ADC callback
 */
static void anaSensorCallback(ADCDriver *adcd, adcsample_t *buf, size_t n)
{
    SENSORS.temp[0] = buf[2];
    SENSORS.temp[1] = buf[3];
    SENSORS.press = buf[1];
    SENSORS.humd = buf[0];
    // TODO: Format and store analog sensor data
}


//static ADCConfig adccfg = {};
static adcsample_t accSamples[ACC_ADC_CH_NUM];
static adcsample_t sensorSamples[SENSOR_ADC_CH_NUM];

/* Accelerometer ADC conversion group */
static const ADCConversionGroup accelConvGrp = 
{
    FALSE,					// Circular Buffer Mode
    ACC_ADC_CH_NUM,				// Number of channels in group
    accCallback,				// Read Callback
    NULL,					// Error Callback
    0,						// ADC CR1
    0, 						// ADC CR2
    0,						// ADC_SMPR1
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_239P5) |	// ADC SMPR2
    ADC_SMPR2_SMP_AN1(ADC_SAMPLE_239P5) |
    ADC_SMPR2_SMP_AN4(ADC_SAMPLE_239P5),
    ADC_SQR1_NUM_CH(ACC_ADC_CH_NUM),		// ADC SQR1						// ADC SQR1
    0,						// ADC SQR2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0) |		// ADC SQR3
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN4)
};

/* Analog sensor ADC configuration group */
static const ADCConversionGroup sensorConvGrp =
{
    FALSE,					// Circular Buffer Mode
    SENSOR_ADC_CH_NUM,				// Number of channels
    anaSensorCallback,				// Read Callback
    NULL,					// Error Callback
    0,						// ADC CR1
    0, 						// ADC CR2
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_239P5)  |	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_239P5)  |
    ADC_SMPR1_SMP_AN13(ADC_SAMPLE_239P5),
    ADC_SMPR2_SMP_AN8(ADC_SAMPLE_239P5),		// ADC SMPR2
    ADC_SQR1_NUM_CH(SENSOR_ADC_CH_NUM),		// ADC SQR1
    0,						// ADC SQR2					
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN8)  |		// ADC SQR3 (Channel Sequence)
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN10) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN11) |
    ADC_SQR3_SQ4_N(ADC_CHANNEL_IN13)
};

/* VCP Serial configuration, 57k6, 8N1 */
static SerialConfig serCfg = 
{
   19200,
   0,
   0,
   0,
};

/* GPS Serial configuration 4k8, 8N1*/
static SerialConfig serGpsCfg =
{
    4800,
    0,
    0,
    0,
};


/**
 * @brief Initialization routine for OS and peripherals
 */
void initialize()
{
    halInit();    	// ChibiOS HAL initialization
    chSysInit();	// ChibiOS System Initialization

    /* 
     * Configure I/O 
     */
    // LED
    palSetPadMode(GPIOA, GPIOA_LED_GREEN, PAL_MODE_OUTPUT_PUSHPULL); // LED
    
    // Serial I/O Cfg
    palSetPadMode(GPIOA, 2, PAL_MODE_STM32_ALTERNATE_PUSHPULL); // VCP Tx
    palSetPadMode(GPIOA, 3, PAL_MODE_INPUT);			// VCP Rx
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL); // GPS Tx
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT); 			// GPS Rx
    
    // ADC I/O Cfg
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); 		// A1.0 ACC_X
    palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG); 		// A1.1 ACC_Y
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG); 		// A1.4 ACC_Z
    palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG); 		// A1.8 HUMD
    palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG); 		// A1.10 PRESS
    palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG); 		// A1.11 TEMP1
    palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG); 		// A1.13 TEMP2
    
    // SPI/MMC I/O Cfg
    palSetPadMode(GPIOA, 5, PAL_MODE_STM32_ALTERNATE_PUSHPULL);	// PA5 SCK1
    palSetPadMode(GPIOA, 6, PAL_MODE_INPUT); 			// PA6 MISO1
    palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL); // PA7 MOSI1
    palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL);		// PB6 NCS
    
    /*
     * Driver Startup
     */
    
    /* GPS UART Startup */
    sdStart(&SD1, &serGpsCfg);
    
    /* VCP Serial Port Startup */
    sdStart(&SD2, &serCfg);	// Activate VCP USART2 driver
    
    /* SPI/MMC Logger Startup */
    int8_t sdIni, dlIni, lfIni;
    sdIni = sdmmcInitialize(&sd, &MMCD1, &SD2);
    if(sdmmcFSMounted(&sd))
    {
	dlIni = dataLoggerInitialize(&logger, "0:", &sd, &SD2);
    }
    lfIni = logfileNew(&sensorLog, &logger);
    logfileClose(&sensorLog);
    chprintf((BaseSequentialStream *) &SD2, "\nSD Initialization: SD:%d,DL:%d,LF:%d\n",sdIni,dlIni,lfIni);
    /* ADC Startup */
    adcStart(&ADCD1, NULL);      // Activate ADC driver
    
}

void printData(void)
{
    chprintf((BaseSequentialStream *) &SD2, "AccX:%03x   AccY:%03x   AccZ:%03x",
		accSamples[0], accSamples[1], accSamples[2]);
    chprintf((BaseSequentialStream *) &SD2, "   Temp1:%03x   Temp2:%03x   Press:%03x   Humd:%03x\n", 
		SENSORS.temp[0], SENSORS.temp[1], SENSORS.press, SENSORS.humd); 
}

/*
 * Application entry point.
 */
int main(void) 
{
    initialize(); // Initialize OS/Peripherals
    
    uint8_t serialIn;
    uint16_t serialLen;
    
    char buffer[100]; // Buffer for misc things
    
    while (TRUE) 
    {

	// TODO: Check for new GPS NMEA sentence
	
	// Perform sensor ADC reads
	adcConvert(&ADCD1, &accelConvGrp, accSamples, ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	adcConvert(&ADCD1, &sensorConvGrp, sensorSamples, ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	// Write serial data to VCP terminal
	
	printData();
	
	palTogglePad(GPIOA, GPIOA_LED_GREEN);
	// TODO: Write data to SD
	chThdSleepMilliseconds(500);
	
    }
  return 0;
}
