/* ADC Defines */
#define ADC_BUF_DEPTH 1 
#define SENSOR_ADC_CH_NUM 4
#define ACC_ADC_CH_NUM 3
#define ADC_CH_NUM 7

#define NSAMPLES 30
#define SAMPLE_MAX 60

#define S_TEMP1	0
#define S_TEMP2 1
#define S_PRESS 2
#define S_HUMD  3
#define S_ACCX  4
#define S_ACCY  5
#define S_ACCZ  6

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"
#include <chstreams.h>
#include <string.h>

#include "gps.h"
#include "sdmmcsimple.h"
#include "datalogger.h"
#include "ustr.h"

#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdfix.h>

#include "board.h"


static THD_WORKING_AREA(waGps, 1284);	// GPS thread working area

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

// Temporary sample struct
typedef struct
{
    char time[10];	// HH:MM:SS
    char latitude[16];	// DDMMM.MMMMMN
    char longitude[16]; // DDDMMM.MMMMME
    char altitude[15];  // +AAAAA.A	(m)
    char satCount[10];  // SS		(Count)
    char temp1[8]; 	// +TTT.T	(C)
    char temp2[8];	// +TTT.T	(C)
    char pressure[8];	// PPP.PP	(kPa)
    char humidity[8];	// RRR.RR	(%RH)
    char accX[10];	// +XX.X	(g)
    char accY[10];	// +YY.Y	(g)
    char accZ[10];	// +ZZ.Z	(g)
}dataSample_t;

static dataSample_t masterSample;

/**
 * @brief Accelerometer ADC callback
 * @param adcd ADC Driver
 * @param buf ADC result buffer
 * @param n Number of reads
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
 * @param adcd ADC Driver
 * @param buf ADC result buffer
 * @param n Number of reads
 */
static void anaSensorCallback(ADCDriver *adcd, adcsample_t *buf, size_t n)
{
    (void) adcd;
    (void) n;
    SENSORS.temp[0] = buf[2];
    SENSORS.temp[1] = buf[3];
    SENSORS.press = buf[1];
    SENSORS.humd = buf[0];

}


//static ADCConfig adccfg = {};
//static adcsample_t accSamples[ACC_ADC_CH_NUM];
//static adcsample_t sensorSamples[SENSOR_ADC_CH_NUM];

static adcsample_t analogSamples[ADC_CH_NUM];

/* Accelerometer ADC conversion group */
/*
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
*/
/* Analog sensor ADC configuration group */
/*
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
*/

/**
 * New analog sensor ADC conversion group
 */
static const ADCConversionGroup analogGrp =
{
    FALSE,
    ADC_CH_NUM,
    NULL,
    NULL,
    0,						// ADC CR1
    ADC_CR2_SWSTART,				// ADC CR2
    ADC_SMPR1_SMP_AN10(ADC_SAMPLE_56) | 	// ADC SMPR1
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) |
    ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56),
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56) |		// ADC SMPR2
    ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) |
    ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56) |
    ADC_SMPR2_SMP_AN8(ADC_SAMPLE_56),
    ADC_SQR1_NUM_CH(ADC_CH_NUM), 		// ADC SQR1
    ADC_SQR2_SQ7_N(ADC_CHANNEL_IN4),		// ADC SQR2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)  | 		// ADC SQR3
    ADC_SQR3_SQ2_N(ADC_CHANNEL_IN13) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN10) |
    ADC_SQR3_SQ4_N(ADC_CHANNEL_IN8) |
    ADC_SQR3_SQ5_N(ADC_CHANNEL_IN0) |
    ADC_SQR3_SQ6_N(ADC_CHANNEL_IN1)
};

/* VCP Serial configuration, 57k6, 8N1 */
static SerialConfig serCfg = 
{
   460800,
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
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));		// VCP Rx
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); 	// GPS Tx
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); 		// GPS Rx
    
    // ADC I/O Cfg
    palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG); 	// A1.0 ACC_X
    palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG); 	// A1.1 ACC_Y
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG); 	// A1.4 ACC_Z
    palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG); 	// A1.8 HUMD
    palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG); 	// A1.10 PRESS
    palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG); 	// A1.11 TEMP1
    palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG); 	// A1.13 TEMP2
    
    /*
     * Driver Startup
     */
    
    /* Real Time Clock */
    //rtcGetTime(&RTCD1, &timespec);
    
    /* GPS Driver Startup */
    //gpsStart(&UARTD1);
    
    /* VCP Serial Port Startup */
    sdStart(&SD2, &serCfg);	// Activate VCP USART2 driver
    
    /* SPI/MMC Logger Startup */
    
    int8_t sdIni, dlIni, lfIni;
    sdIni = sdmmcInitialize(&sd, &MMCD1, &SD2);
    chThdSleepMilliseconds(250);
    if(!sdIni)
    {
	if(sdmmcFSMounted(&sd))
	{
	    dlIni = dataLoggerInitialize(&logger, "", &sd, &SD2);
	}
	chprintf((BaseSequentialStream *) &SD2, "\nSD Initialization: SD:%d,DL:%d,LF:%d\n",sdIni,dlIni,lfIni);
    }
    else
    {
	chprintf((BaseSequentialStream *) &SD2, "\nSD Initialization Failed\n");
    }
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
    char line[120];
    int len = chsnprintf(line, 120, "%d,%d,%d,%d,%d,%d,%d,%d\n",time, acc->x, acc->y, acc->z, sens->temp[0], sens->temp[1], sens->press, sens->humd);
    logfileWrite(log, line, len, false);
}

/**
 * @brief Writes a master sample header to a logfile
 */
void writeMSHeader(void)
{
    char line[72];
    int len = chsnprintf(line, 72, "TIME,LAT,LONG,ALT,SAT,TMP1,TMP2,PRESS,HUMD,ACCX,ACCY,ACCZ\n");
    logfileWrite(&sensorLog, line, len, false);
}

/**
 * @brief Writes a master sample to the logfile
 * @param log Logfile to write to
 * @param ms Master sample
 * @return Status of writing sample to log
 */
static char dataPrintBuf[384];
int8_t writeMSToLog(dataSample_t *ms)
{
    //chprintf((BaseSequentialStream *) &SD2, "Building string\n");
    //chThdSleepMilliseconds(200);
    
    // Time,Lat,Long,Alt,Sat,Temp1,Temp2,Press,Humd,AccX,AccY,AccZ
    int len = chsnprintf(dataPrintBuf, 256, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
			 (char *)(ms->time),
			 (char *)(ms->latitude),
			 (char *)(ms->longitude),
			 (char *)(ms->altitude),
			 (char *)(ms->satCount),
			 (char *)(ms->temp1),
			 (char *)(ms->temp2),
			 (char *)(ms->pressure),
			 (char *)(ms->humidity),
			 (char *)(ms->accX),
			 (char *)(ms->accY),
			 (char *)(ms->accZ)
    );
    
    //chprintf((BaseSequentialStream *) &SD2, "Str built, writing\n");
    //chThdSleepMilliseconds(200);
    
    logfileWrite(&sensorLog, dataPrintBuf, len, false);
    return 0;
}

/**
 * @brief Opens a new logfile
 * @param lfNum Logfile number
 * @return Status of opening new logfile
 */
int8_t openNewLogfile(int32_t lfNum)
{
    if(!sensorLog.open)
    {
	char fname[24];
	if(lfNum < 10)
	    chsnprintf(fname, 80, "logs/LOG_00%d.csv", lfNum);
	else if(lfNum < 100)
	    chsnprintf(fname, 80, "logs/LOG_0%d.csv", lfNum);
	else
	    chsnprintf(fname, 80, "logs/LOG_%d.csv", lfNum);
	//chsnprintf(fname, 80, "logs/Log_00X.csv");
	return logfileNew(&sensorLog, &logger, &lFile, fname); 
    }
    return 2;
}

/**
 * @brief Convenience function to close the current sensor log
 */
int8_t closeLog(void)
{
    logfileClose(&sensorLog);
    return 0;
}

/**
 * @brief Prints ADC data to debug serial port
 */
/*
void printData(void)
{
    chprintf((BaseSequentialStream *) &SD2, "AccX:%03x   AccY:%03x   AccZ:%03x",
		accSamples[0], accSamples[1], accSamples[2]);
    chprintf((BaseSequentialStream *) &SD2, "   Temp1:%03x   Temp2:%03x   Press:%03x   Humd:%03x\n", 
		SENSORS.temp[0], SENSORS.temp[1], SENSORS.press, SENSORS.humd); 
}
*/

/**
 * @brief Prints GPS Data to the serial port
 */
void printGps(gpsLocation_t *loc)
{
    chprintf((BaseSequentialStream *) &SD2, "TIME:%s, LAT:%s, LONG:%s, ALT:%s, SAT:%s\n",
	     (char *)(loc->time),
	     (char *)(loc->latitude),
	     (char *)(loc->longitude),
	     (char *)(loc->altitude),
	     (char *)(loc->satCount)
    );
}

/**
 * @brief Print a data sample to the terminal
 */
void printDataSample(dataSample_t *samp)
{
   chprintf((BaseSequentialStream *) &SD2, "TIM:%s, LAT:%s, LNG:%s, ALT:%s, SAT:%s, TMP1:%s, TMP2:%s, PRS:%s, HUM:%s, ACC(%s,%s,%s)\n",
	    (char *)(samp->time),
	    (char *)(samp->latitude),
	    (char *)(samp->longitude),
	    (char *)(samp->altitude),
	    (char *)(samp->satCount),
	    (char *)(samp->temp1),
	    (char *)(samp->temp2),
	    (char *)(samp->pressure),
	    (char *)(samp->humidity),
	    (char *)(samp->accX),
	    (char *)(samp->accY),
	    (char *)(samp->accZ)
   );
}

/**
 * @brief Copies GPS data into a data sample for writing
 * @param loc GPS Location
 * @param samp Data sample to store to
 */
void gpsToDataSample(gpsLocation_t *loc, dataSample_t *samp)
{
    uStrCpy(samp->time, loc->time); // Copy
    uStrCpy(samp->latitude, loc->latitude);
    uStrCpy(samp->longitude, loc->longitude);
    uStrCpy(samp->altitude, loc->altitude);
    uStrCpy(samp->satCount, loc->satCount);
}

/**
 * @brief Formats temperature data and stores it to a data sample for writing
 * @param rawTemp Raw temperature ADC value
 * @param tempNum Temperature sensor number
 * @param samp Data sample to store to
 */
void tempToDataSample(int16_t rawTemp, uint8_t tempNum, dataSample_t *samp)
{
    // 10mV/C Absolute output 750mV@25C
    float temp = (1.0f * rawTemp - 620.454545f) /12.4090909f; // Convert to C
    char *dest;
    switch(tempNum)
    {
	case 0:
	    dest = (char *) (samp->temp1);
	    break;
	case 1:
	    dest = (char *) (samp->temp2);
	    break;
	default:
	    return;
    }
    chsnprintf(dest, 6, "%.2f" ,temp);
}

/**
 * @brief Formats pressure data and stores it to a data sample for writing
 * @param rawPressure Raw pressure ADC value
 * @param samp Data sample to store to
 */
void pressureToDataSample(int16_t rawPressure, dataSample_t *samp)
{
    // 0.1224V offset (nominal), 27.0588235 mv/kPa
    float press = (1.0f*rawPressure + 405.234375f)/38.390625f; // Convert to kPa
    chsnprintf((char *) (samp->pressure), 6, "%.2f", press);
}

/**
 * @brief Formats humidity data and stores it to a data sample for writing
 * @param rawHumidity Raw humidity ADC value
 * @param samp Data sample to store to
 */
void humidityToDataSample(int16_t rawHumidity, dataSample_t *samp)
{
    // Ratiometric output, 47k/100k divider, RH = ((raw/4095)-0.16)/(0.0062)
    float hum = (1.0f * rawHumidity - 675.324675f)/26.1688312f; // Convert to %RH
    chsnprintf(samp->humidity, 6, "%.2f", hum); //RRR.RR
}

/**
 * @brief Formats accelerometer data and stores it to a data sample for writing
 * @param rawAccel Raw accelerometer ADC value
 * @param xyz X(0), Y(1), Z(2) channel of the accelerometer
 * @param samp Data sample to store to
 */
void accelToDataSample(int16_t rawAccel, int8_t xyz, dataSample_t *samp)
{
    // Ratiometric output (10%/g) center: Vcc/2
    float accel = (2047.0f - rawAccel)/409.5f; //Convert to g
    char *dest;
    switch(xyz)
    {
	case 0:
	    dest = (char *) (samp->accX);
	    break;
	case 1:
	    dest = (char *) (samp->accY);
	    break;
	case 2:
	    dest = (char *) (samp->accZ);
	    break;
	default:
	    return;
	    break;
    }
    chsnprintf(dest, 9, "%.5f", accel);
}

/**
 * @brief Initializes the master data sample
 */
void initMasterSample()
{
    char * emptyStr = "-";
    uStrCpy(masterSample.time, "-:-:-");
    uStrCpy(masterSample.latitude, emptyStr);
    uStrCpy(masterSample.longitude, emptyStr);
    uStrCpy(masterSample.altitude, emptyStr);
    uStrCpy(masterSample.satCount, emptyStr);
    uStrCpy(masterSample.temp1, emptyStr);
    uStrCpy(masterSample.temp2, emptyStr);
    uStrCpy(masterSample.pressure, emptyStr);
    uStrCpy(masterSample.humidity, emptyStr);
    uStrCpy(masterSample.accX, emptyStr);
    uStrCpy(masterSample.accY, emptyStr);
    uStrCpy(masterSample.accZ, emptyStr);
}

/**
 * Application entry point.
 */
int main(void) 
{
    initialize(); // Initialize OS/Peripherals
    initMasterSample();
    
    gpsThread_t gpsThd;
    
    uint32_t timeCounter = 0;
    //chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO, gpsThread, &gpsThd);
    
    uint16_t logfileCounter = 0; // Logfile number counter
    uint32_t sampleCounter = 0;
    
    gpsLocation_t location;
    
    openNewLogfile(logfileCounter++);
    writeMSHeader();
    while (TRUE) 
    {
	//gpsGetLocation(&location);// Check for new GPS NMEA sentence
	//printGps(&location);
	// Perform sensor ADC reads
	//adcConvert(&ADCD1, &accelConvGrp, accSamples, ADC_BUF_DEPTH);
	
	adcConvert(&ADCD1, &analogGrp, analogSamples, ADC_BUF_DEPTH);
	
	accelToDataSample(analogSamples[S_ACCX], 0, &masterSample);
	accelToDataSample(analogSamples[S_ACCY], 1, &masterSample);
	accelToDataSample(analogSamples[S_ACCZ], 2, &masterSample);
	
	chThdSleepMilliseconds(10);
	//adcConvert(&ADCD1, &sensorConvGrp, sensorSamples, ADC_BUF_DEPTH);
	tempToDataSample(analogSamples[S_TEMP1], 0, &masterSample);
	tempToDataSample(analogSamples[S_TEMP2], 1, &masterSample);
	pressureToDataSample(analogSamples[S_PRESS], &masterSample);
	humidityToDataSample(analogSamples[S_HUMD], &masterSample);
	
	chThdSleepMilliseconds(10);
	
	// Write GPS Data to MasterSample
	gpsToDataSample(&location, &masterSample);
	
	// Write sensor data to MasterSample

	// Write serial data to VCP terminal
	//printData();
	printDataSample(&masterSample);
	
	//writeToLog(&sensorLog, timeCounter, &SENSORS, &ACC);
	
	toggleLED();
	chThdSleepMilliseconds(975);
	
	if(sampleCounter > SAMPLE_MAX) // Close file and open a new one
	{
	    chprintf((BaseSequentialStream *) &SD2, "Reached sample limit, closing current logfile\n");
	    closeLog();
	    openNewLogfile(logfileCounter++);
	    writeMSHeader();
	    sampleCounter = 0;
	}
	//chprintf((BaseSequentialStream *) &SD2, "Writing sample data to log\n");
	writeMSToLog(&masterSample);
	//chprintf((BaseSequentialStream *) &SD2, "Log written\n");
	sampleCounter++;
    }
  return 0;
}
