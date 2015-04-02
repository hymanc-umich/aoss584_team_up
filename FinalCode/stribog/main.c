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
#include "chvt.h"
#include "chprintf.h"
#include "ff.h"
#include <chstreams.h>
#include <string.h>

#include "gps.h"
#include "sdmmcsimple.h"
#include "datalogger.h"
#include "ustr.h"
#include "sensor_thread.h"

#include "Drivers/xbeepro.h"

#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "board.h"
#include "datasample.h"


static THD_WORKING_AREA(waGps, 1284);	 // GPS thread working area
static THD_WORKING_AREA(waSensor, 4096); // Sensor thread working area
static THD_WORKING_AREA(waCom, 4096);	 // Communication thread
//static THD_WORKING_AREA(waSD, 1024); 	 // SD thread

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


// Data Logging
static dataSample_t masterSample;
static sdmmc_t sd;
static datalogger_t logger;
static logfile_t sensorLog;
FIL lFile;
MMCDriver MMCD1;


static xbeePro_t xbee;

/* Debug Serial configuration, 460k8, 8N1 */
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
     * Configure I/O : DEPRECATED
     */
    //boardInit();
    
    chThdSleepMilliseconds(100); // Startup wait (may be a bit long)
    /*
     * Driver Startup
     */
    
    /* Real Time Clock */
    //rtcGetTime(&RTCD1, &timespec);
    
    /* GPS Driver Startup */
    gpsStart(&GPS_UART);
    
    /* Debug Serial Port Startup */
    //sdStart(&COM_SERIAL, &serCfg);
    sdStart(&DBG_SERIAL, &serCfg);	// Activate Debug serial driver
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "==Stribog v1==\n");

    /* SPI/MMC Logger Startup */
    int8_t sdIni, dlIni, lfIni;
    
    //chThdSleepMilliseconds(200); // Wait for SD startup

    // XBee Initialization
    xbeePro_init(&xbee, &COM_SERIAL);
    /* // SD
    sdIni = sdmmcInitialize(&sd, &MMCD1, &DBG_SERIAL);
   	chprintf((BaseSequentialStream *) &DBG_SERIAL, "Initializing Datalogger\n");
    chThdSleepMilliseconds(100);
    if(!sdIni)
    {
    	if(sdmmcFSMounted(&sd))
    	{
    	    dlIni = dataLoggerInitialize(&logger, "", &sd, &DBG_SERIAL);
    	}
    	//chprintf((BaseSequentialStream *) &DBG_SERIAL, "\nSD Initialization: SD:%d,DL:%d,LF:%d\n",sdIni,dlIni,lfIni);
    }
    else
    {
	   chprintf((BaseSequentialStream *) &DBG_SERIAL, "\nERROR: SD Initialization Failed\n");
    }
    */
    
    /* ADC Startup */
    adcStart(&ADCD1, NULL);      // Activate ADC driver
    
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
	    chsnprintf(fname, 80, "logs/DAT_00%d.csv", lfNum);
	else if(lfNum < 100)
	    chsnprintf(fname, 80, "logs/DAT_0%d.csv", lfNum);
	else
	    chsnprintf(fname, 80, "logs/DAT_%d.csv", lfNum);
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
 * @brief Prints GPS Data to the serial port
 */
void printGps(gpsLocation_t *loc)
{
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "TIME:%s, LAT:%s, LONG:%s, ALT:%s, SAT:%s\n",
	     (char *)(loc->time),
	     (char *)(loc->latitude),
	     (char *)(loc->longitude),
	     (char *)(loc->altitude),
	     (char *)(loc->satCount)
    );
}

/**
 * @brief Writes a master sample header to a logfile
 * @return 
 */
int8_t writeHeader(void)
{
    char line[72];
    int len = chsnprintf(line, 72, TELEMETRY_HEADER);
    return logfileWrite(&sensorLog, line, len, false);
}

/**
 * Application entry point.
 */
int main(void) 
{
    initialize(); // Initialize OS/Peripherals
    //datasample_init(&masterSample);
    
    gpsThread_t gpsThd;
    sensorThread_t sensorThd;
    
    uint32_t timeCounter = 0;
    chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO, gpsThread, &gpsThd); 			// Create GPS Thread
    chThdCreateStatic(waSensor, sizeof(waSensor), NORMALPRIO, sensorThread, &sensorThd);	// Create sensor thread
       
    uint16_t logfileCounter = 0; 	// Logfile number counter
    uint32_t sampleCounter = 0;		// Sample counter (resets per file)
    
    gpsLocation_t location;		// GPS location
       
    // Startup chirp
    boardSetBuzzer(1);
    boardSetLED(1);
    //chThdSleepMilliseconds(10);
    boardSetBuzzer(0);
    boardSetLED(0);
    
    //openNewLogfile(logfileCounter++);
    //writeHeader();

    // 1Hz Master Loop
    systime_t deadline = chVTGetSystemTimeX();
    while (TRUE) 
    {
    	deadline += MS2ST(1000); 
		//boardSetLED(1);
		gpsGetLocation(&location);// Check for new GPS NMEA sentence
		printGps(&location);
		chprintf((BaseSequentialStream *) &DBG_SERIAL, "LOOP\n");
		// Read out I2C sensors
		// Read out Analog sensors
		// Post data
		
		
		chprintf((BaseSequentialStream *) &DBG_SERIAL, "\n");		
		// Write GPS Data to MasterSample
		//datasample_gpsToSample(&location, &masterSample);
		
		// Write sensor data to MasterSample

		// Write serial data to VCP terminal
		//printData();
		//printDataSample(&masterSample);
		
		//writeToLog(&sensorLog, timeCounter, &SENSORS, &ACC);
		
		//chThdSleepMilliseconds(975);
		/*
		if(sampleCounter > SAMPLE_MAX) // Close file and open a new one
		{
		    chprintf((BaseSequentialStream *) &DBG_SERIAL, "Reached sample limit, closing current logfile\n");
		    closeLog();
		    openNewLogfile(logfileCounter++);
		    writeHeader();
		    sampleCounter = 0;
		}
		//chprintf((BaseSequentialStream *) &DBG_SERIAL, "Writing sample data to log\n");
		datasample_writeToLog(&masterSample, &sensorLog);
		//chprintf((BaseSequentialStream *) &DBG_SERIAL, "Log written\n");
		sampleCounter++;
		*/
		if(chVTGetSystemTimeX() < deadline)
			chThdSleepUntil(deadline);
	}
	return 0;
}
