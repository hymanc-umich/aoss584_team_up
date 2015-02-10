#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include <chstreams.h>
#include <string.h>

#include "gps.h"
#include "sdmmcsimple.h"
#include "datalogger.h"

#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/* ADC Defines */
#define ADC_BUF_DEPTH 1 // Double buffered ADC
#define SENSOR_ADC_CH_NUM 4
#define ACC_ADC_CH_NUM 3

/* LED Define */
#define LED_PORT GPIOA
#define LED_PIN 10

#define NSAMPLES 30

static THD_WORKING_AREA(waGps, 512);

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

static sdmmc_t sd;
static datalogger_t logger;
static logfile_t sensorLog;
FIL lFile;

// ADC measurement
static accMeasurement_t ACC;
static sensorMeasurement_t SENSORS;

MMCDriver MMCD1;

/**
 * @brief Accelerometer ADC callback
 */
static void accCallback(ADCDriver *adcd, adcsample_t *buf, size_t n)
{
    (void) adcd;
    (void) n;
    ACC.x = buf[0];
    ACC.y = buf[1];
    ACC.z = buf[2];
}

/**
 * @brief Analog sensor ADC callback
 */
static void anaSensorCallback(ADCDriver *adcd, adcsample_t *buf, size_t n)
{
    (void) adcd;
    (void) n;
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
    ADC_CR2_SWSTART, 				// ADC CR2
    0,						// ADC_SMPR1
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56) |		// ADC SMPR2
    ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) |
    ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56),
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
    ADC_CR2_SWSTART, 				// ADC CR2
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_56)  |	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56)  |
    ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56),
    ADC_SMPR2_SMP_AN8(ADC_SAMPLE_56),		// ADC SMPR2
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

/**
 * @brief Initialization routine for OS and peripherals
 */
void initialize(void)
{
    halInit();    	// ChibiOS HAL initialization
    chSysInit();	// ChibiOS System Initialization

    /* 
     * Configure I/O 
     */
    // LED
    palSetPadMode(LED_PORT, LED_PIN, PAL_MODE_OUTPUT_PUSHPULL); // LED
    
    // Serial I/O Cfg
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7)); 	// VCP Tx
    palSetPadMode(GPIOA, 3, PAL_MODE_INPUT);		// VCP Rx
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); 	// GPS Tx
    palSetPadMode(GPIOA, 10, PAL_MODE_INPUT); 		// GPS Rx
    
    // ADC I/O Cfg
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); 	// A1.0 ACC_X
    palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG); 	// A1.1 ACC_Y
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG); 	// A1.4 ACC_Z
    palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG); 	// A1.8 HUMD
    palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG); 	// A1.10 PRESS
    palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG); 	// A1.11 TEMP1
    palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG); 	// A1.13 TEMP2
    
    // SPI/MMC I/O Cfg
    palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);	// PA5 SCK1
    palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST); 	// PA6 MISO1
    palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST); 	// PA7 MOSI1
    palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);	// PB6 NCS
    
    /*
     * Driver Startup
     */
    
    /* Real Time Clock */
    //rtcGetTime(&RTCD1, &timespec);
    
    /* GPS Driver Startup */
    gpsStart(&UARTD1);
    
    /* VCP Serial Port Startup */
    sdStart(&SD2, &serCfg);	// Activate VCP USART2 driver
    
    /* SPI/MMC Logger Startup */
    int8_t sdIni, dlIni, lfIni;
    sdIni = sdmmcInitialize(&sd, &MMCD1, &SD2);
    if(sdmmcFSMounted(&sd))
    {
	dlIni = dataLoggerInitialize(&logger, "", &sd, &SD2);
    }
    lfIni = logfileNew(&sensorLog, &logger, &lFile, "logs/log_001.csv");
    logfileWrite(&sensorLog, "TIME,ACC.X,ACC.Y,ACC.Z,TEMP1,TEMP2,PRESSURE,HUMIDITY\n", 53, false);
    //logfileWrite(&sensorLog, "THIS IS A TEST\n",15); 
    //logfileClose(&sensorLog);
    chprintf((BaseSequentialStream *) &SD2, "\nSD Initialization: SD:%d,DL:%d,LF:%d\n",sdIni,dlIni,lfIni);
    /* ADC Startup */
    adcStart(&ADCD1, NULL);      // Activate ADC driver
    
}

/**
 * @brief Writes data to log file
 * @param log Logfile to write to
 * @param time Time to write
 * @param sens Environmental sensor data
 * @param acc Accelerometer data
 */
void writeToLog(logfile_t *log, uint32_t time, sensorMeasurement_t *sens, accMeasurement_t *acc)
{
    char line[80];
    int len;
    len = chsnprintf(line, 80, "%d,%d,%d,%d,%d,%d,%d,%d\n",time, acc->x, acc->y, acc->z, sens->temp[0], sens->temp[1], sens->press, sens->humd);
    logfileWrite(log, line, len, false);
}

/**
 * @brief Prints ADC data to debug serial port
 */
void printData(void)
{
    chprintf((BaseSequentialStream *) &SD2, "AccX:%03x   AccY:%03x   AccZ:%03x",
		accSamples[0], accSamples[1], accSamples[2]);
    chprintf((BaseSequentialStream *) &SD2, "   Temp1:%03x   Temp2:%03x   Press:%03x   Humd:%03x\n", 
		SENSORS.temp[0], SENSORS.temp[1], SENSORS.press, SENSORS.humd); 
}


/**
 * Application entry point.
 */
int main(void) 
{
    initialize(); // Initialize OS/Peripherals
    gpsThread_t gpsThd;
    
    
    uint32_t timeCounter = 0;
    chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO, gpsThread, &gpsThd);
    
    gpsLocation_t location;
    while (TRUE) 
    {
	gpsGetLocation(&location);// TODO: Check for new GPS NMEA sentence
	
	// Perform sensor ADC reads
	adcConvert(&ADCD1, &accelConvGrp, accSamples, ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	adcConvert(&ADCD1, &sensorConvGrp, sensorSamples, ADC_BUF_DEPTH);
	chThdSleepMilliseconds(10);
	// Write serial data to VCP terminal
	
	printData();
	writeToLog(&sensorLog, timeCounter, &SENSORS, &ACC);
	writeToLog(&sensorLog, timeCounter, &SENSORS, &ACC);
	//logfileWrite(&sensorLog, "THIS IS A TEST\n",15); 
	toggleLED();
	chThdSleepMilliseconds(250);
	if(timeCounter++ >= NSAMPLES)
	{
	    chprintf((BaseSequentialStream *) &SD2, "CLOSING FILE\n");
	    logfileClose(&sensorLog);
	    break;
	}
    }
  return 0;
}
