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

#include "watchdog.h"

#define SAMPLE_MAX 60

static THD_WORKING_AREA(waBeacon, 64);   // Audio Beacon thread
static THD_WORKING_AREA(waGps, 1284);	 // GPS thread working area
static THD_WORKING_AREA(waSensor, 4096); // Sensor thread working area

/*===== Data Logging System Statics =====*/
static dataSample_t masterSample;   // Data sample struct
static sdmmc_t sd;                  // SD/MMC driver wrapper struct
static datalogger_t logger;         // Datalogger struct
static logfile_t sensorLog;         // Sensorlog struct
FIL lFile;                          // Logfile allocation
MMCDriver MMCD1;                    // MMC-SPI Driver

static uint16_t lfNum = 0;

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

    chThdSleepMilliseconds(30); // Startup wait
   
    /*
     * Driver Startup
     */
    
    /* Real Time Clock */
    //rtcGetTime(&RTCD1, &timespec);
    
    /* GPS Driver Startup */
    gpsStart(&GPS_UART);
    
    /* ===== Debug Serial Port Startup ===== */
    sdStart(&DBG_SERIAL, &serCfg);	// Activate Debug serial driver
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "\n\n=== Stribog v1 ===\n(C) 2015, Cody Hyman\n\nInitializing System...\n");
    chprintf((BaseSequentialStream *) &COM_SERIAL, "\n\n=== Stribog v1 ===\n(C) 2015, Cody Hyman\n\nInitializing System...\n");
    chThdSleepMilliseconds(250); // Wait for SD startup

    /* ===== XBee Initialization ===== */
    xbeePro_init(&xbee, &COM_SERIAL);

    /* ===== SPI/MMC Logger Startup ===== */
    int8_t sdIni, dlIni;
    sdIni = sdmmcInitialize(&sd, &MMCD1, &DBG_SERIAL);
   	chprintf((BaseSequentialStream *) &DBG_SERIAL, "Initializing Datalogger FS\n");
    chThdSleepMilliseconds(10);
    if(!sdIni)
    {
    	if(sdmmcFSMounted(&sd))
    	{
    	    dlIni = dataLoggerInitialize(&logger, "", &sd, &DBG_SERIAL);
    	}
        else
        {
            dlIni = -99;
        }
    	chprintf((BaseSequentialStream *) &DBG_SERIAL, "\nSD Initialization: SD:%d,DL:%d\n",sdIni,dlIni);
    }
    else
    {
	   chprintf((BaseSequentialStream *) &DBG_SERIAL, "\nERROR: SD Initialization Failed\n");
    }

    int16_t fcount = 0;
    FRESULT fcstatus = dataLoggerFileCount(&logger, "LOGS", &fcount);
    if(fcstatus == FR_OK)
    {
        lfNum = fcount + 1;
        chprintf((BaseSequentialStream *) &COM_SERIAL,"FILECOUNT: %d\n",fcount);
    }
    else
        chprintf((BaseSequentialStream *) &COM_SERIAL, "FILECOUNT ERROR: %d\n",fcstatus);
    //chThdSleepMilliseconds(2000);
    /* ===== ADC Startup ===== */
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "Initializing ADC\n");
    adcStart(&ADCD1, NULL);      // Activate ADC driver
    watchdog_init(IWDG_PRESCALER_DIV256,256,true); // Initialize watchdog
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "System Initialization Complete\n");
}


/**
 * @brief Opens a new logfile
 * @return Status of opening new logfile
 */
int8_t openNewLogfile(void)
{
    int8_t status = -99;
    if(!sensorLog.open)
    {
        int8_t status;
        lfNum++;
    	char fname[36];
    	if(lfNum < 10)
    	    chsnprintf(fname, 80, "LOGS/DAT00%d.csv", lfNum);
    	else if(lfNum < 100)
    	    chsnprintf(fname, 80, "LOGS/DAT0%d.csv", lfNum);
    	else
    	    chsnprintf(fname, 80, "LOGS/DAT%d.csv", lfNum);
    	status = logfileNew(&sensorLog, &logger, &lFile, fname); 
        if(status != 0)
        {
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "ERROR: Could not open new logfile (%d)\n", status);
            lfNum--;    // Undo filenumber increment
        }
    }
    return status;
}

/**
 * @brief Convenience function to close the current sensor log
 */
int8_t closeLog(void)
{
    int8_t status = -99;
    uint8_t i;
    if(sensorLog.open)
    {
        for(i = 0; i < 10; i++)
        {
            status = logfileClose(&sensorLog);
            if(status == FR_OK)
                return status;
            chThdSleepMilliseconds(2);
        }
    }
    return status;
}

/**
 * @brief Prints GPS Data to the serial port
 */
void printGps(BaseSequentialStream *stream, gpsLocation_t *loc)
{
    chprintf(stream, "<GPS>TIME:%s, LAT:%s, LONG:%s, ALT:%s, SAT:%s</GPS>\n",
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
    char line[512];
    int len = chsnprintf(line, 512, TELEMETRY_HEADER);
    return logfileWrite(&sensorLog, line, len, FALSE);
}

/**
* @brief Thread for audio beacon
*
*
*/
msg_t beaconThread(void *args)
{
    bool *beaconFlag = (bool *) args;
    while(TRUE)
    {
        if(*beaconFlag)
        {
            boardSetBuzzer(1);
            chThdSleepMilliseconds(200);
            boardSetBuzzer(0);
            chThdSleepMilliseconds(9900);
        }
        else
        {
            chThdSleepMilliseconds(500);
        }
    }
    return MSG_OK;
}

/**
 * Application entry point.
 */
int main(void) 
{
    initialize(); // Initialize OS/Peripherals
    datasample_init(&masterSample);
    gpsThread_t gpsThd;
    sensorThread_t sensorThd;
    bool audioBeaconFlag;
    uint32_t timeCounter = 0;

    uint8_t i;
    int8_t lfStatus;
    for(i = 0; i < 5; i++)
    {
        lfStatus = openNewLogfile();
        if(lfStatus != -99)
            break;
        else
        {
            //chprintf((BaseSequentialStream *) &DBG_SERIAL, "Previous log still open, retrying\n");
            chThdSleepMilliseconds(10);
        }
    }
    chThdSleepMilliseconds(1000);
    writeHeader();
    if(lfStatus == 0)
    {
         //writeHeader();
         //chprintf((BaseSequentialStream *) &DBG_SERIAL, "New logfile header written\n");
    }
    else
    {
        //chprintf((BaseSequentialStream *) &DBG_SERIAL, "Error opening new logfile: %d\n", lfStatus);
    }
    chThdSleepMilliseconds(300);

    chThdCreateStatic(waGps, sizeof(waGps), NORMALPRIO, gpsThread, &gpsThd); 			// Create GPS Thread
    chThdCreateStatic(waSensor, sizeof(waSensor), NORMALPRIO, sensorThread, &sensorThd);	// Create sensor thread
    chThdCreateStatic(waBeacon, sizeof(waBeacon), NORMALPRIO, beaconThread, &audioBeaconFlag);
    //uint16_t logfileCounter = 0; 	// Logfile number counter
    uint32_t sampleCounter = 0;		// Sample counter (resets per file)
    
    gpsLocation_t location;		    // GPS location
       
    // Startup chirp
    boardSetBuzzer(1);
    boardSetLED(1);
    //chThdSleepMilliseconds(10);
    boardSetBuzzer(0);
    boardSetLED(0);

    // 1Hz Master Loop
    systime_t deadline = chVTGetSystemTimeX();
    while (TRUE) 
    {
    	deadline += MS2ST(1000); 
		boardSetLED(1);
		gpsGetLocation(&location);    // Check for new GPS NMEA sentence
		printGps((BaseSequentialStream *) &COM_SERIAL, &location);
        //printGps((BaseSequentialStream *) &DBG_SERIAL, &location);
        sensorThread_publishData(&sensorThd, (BaseSequentialStream *) &COM_SERIAL);
        //chprintf((BaseSequentialStream *) &DBG_SERIAL, "LOOP\n");

		// Write GPS Data to MasterSample
		datasample_gpsToSample(&location, &masterSample);
		datasample_sensorsToSample(&(sensorThd.data),&masterSample);
		// TODO: Write sensor data to MasterSample
		
        /* ===== Data file handling ===== */
		if(sampleCounter >= SAMPLE_MAX) // Close file and open a new one
		{
    	    //chprintf((BaseSequentialStream *) &DBG_SERIAL, "Reached sample limit, closing current logfile\n");
    	    closeLog();
            chThdSleepMilliseconds(70);
    	    int8_t newLogStatus = openNewLogfile();
            //chprintf((BaseSequentialStream *) &DBG_SERIAL, "\n\nLOGFILE: New file status%d\n\n", newLogStatus);
            sampleCounter = 0;
            writeHeader();
            /*if(newLogStatus == 0)
            {
    	      writeHeader();
              //sampleCounter = 0;
            }*/
		}
		//chprintf((BaseSequentialStream *) &DBG_SERIAL, "Writing sample data to log\n");
		int8_t lfWriteStatus = datasample_writeToLog(&masterSample, &sensorLog);
        if(lfWriteStatus)
            chprintf((BaseSequentialStream *) &DBG_SERIAL, "Error Writing to Log:%d\n",lfWriteStatus);
        else
		  chprintf((BaseSequentialStream *) &DBG_SERIAL, "Log written\n");
		sampleCounter++;
		
        timeCounter++; // Increment time counter
        if(timeCounter > 2500)
            audioBeaconFlag = TRUE;
        // Sleep for remaining slack (to 1s)
        watchdog_throwBone();
		if(chVTGetSystemTimeX() < deadline)
			chThdSleepUntil(deadline);
	}
	return 0;
}
