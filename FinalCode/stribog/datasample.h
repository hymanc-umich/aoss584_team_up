#ifndef _DATA_SAMPLE_H_
#define _DATA_SAMPLE_H_

#include "ch.h"
#include "datalogger.h"
#include "sensor_thread.h"
#include "ustr.h"
#include "gps.h"

static char *TELEMETRY_HEADER = "SN,GPS_TIME,GPS_LAT,GPS_LONG,GPS_ALT,GPS_SAT,TEMP_TMP275,TEMP_SI7020I,HUM_SI7020I,TEMP_BMP280,PRESS_BMP280,TEMP_SI7020E,HUM_SI7020E,TEMP_HIH6030,HUM_HIH6030,TEMP_MS5607,PRESS_MS5607,PRESS_MPXM,TEMP_RTD,VIN,AX,AY,AZ,MX,MY,MZ\n";

// GPS location sample struct
typedef struct
{
    char latitude[16];	// DDMMM.MMMMMN
    char longitude[16]; // DDDMMM.MMMMME
    char altitude[15];  // +AAAAA.A	(m)
    char satCount[10];  // SS		(Count)
}gpsSample_t;

// 3-vector sample struct
typedef struct
{
    char x[14];
    char y[14];
    char z[14];
}vector3Sample_t;

// Data sample struct
/*
typedef struct
{
    uint32_t sn;            // Sample number
    char time[10];		    // HH:MM:SS
    gpsSample_t gps;		// GPS subsample
    char tempBMP[9]; 		// +TTT.TT	(C)
    char pressBMP[10];		// PPP.PPP  (kPa)
    char i7020Humd[7];	    // HHH.H    (%RH)
    char i7020Temp[9];      // +TTT.TT  (C)
    char e7020Humd[7];      // HHH.H    (%RH)
    char e7020Temp[9];      // +TTT.TT  (C)
    char ms5607Press[9];	// PPP.PPP  (kPa)
    char ms5607Temp[9];     // +TTT.TT  (C)
    char pressMpxm[8];      // PPP.PP   (kPa)
    char hih6030Humd[8];    // HHH.HH   (%RH)
    char hih6030Temp[8];    // +TTT.T   (C)
    char extTemp275[8];	    // +TTT.T Ext Dig. temperature (C)
    char extTempRTD[9];	    // +TTT.TT RTD temperature (C)
    char vin[6];            // V.VV
    vector3Sample_t acc;	// Accelerometer subsample (m/s^2)
    vector3Sample_t mag;	// Magnetometer subsample (g)
}dataSample_t;
*/

typedef struct
{
    uint32_t sn;
    char time[10];
    gpsSample_t gps;
    float bmp280Temp;
    float bmp280Press;
    float i7020Humd;
    float i7020Temp;
    float e7020Humd;
    float e7020Temp;
    float ms5607Temp;
    float ms5607Press;
    float mpxmPress;
    float hih6030Humd;
    float hih6030Temp;
    float tmp275Temp;
    float rtdTemp;
    float vin;
    float acc[3];
    float mag[3];
}dataSample_t;

void datasample_init(dataSample_t *s);
int8_t datasample_writeToLog(dataSample_t *s, logfile_t *log);
void datasample_gpsToSample(gpsLocation_t *loc, dataSample_t *samp);
void datasample_sensorsToSample(sensorData_t *sensors, dataSample_t *samp);

#endif