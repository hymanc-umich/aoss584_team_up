#ifndef _DATA_SAMPLE_H_
#define _DATA_SAMPLE_H_

#include "ch.h"
#include "datalogger.h"
#include "ustr.h"
#include "gps.h"

static char * TELEMETRY_HEADER = "SN,TIME,LAT,LONG,ALT,SAT,TMPI,TMPE1,TMPE2,PRESSI,PRESSE1,PRESSE2,HUMDI,HUMDE1,HUMDE2,ACCX,ACCY,ACCZ,MAGX,MAGY,MAGZ\n";

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
typedef struct
{
    uint32_t sn;            // Sample number
    char time[10];		    // HH:MM:SS
    gpsSample_t gps;		// GPS subsample
    char tempBMP[8]; 		// +TTT.T	(C)
    char pressBMP[8];		//
    char intHumidity[8];		// +TTT.T	(C)
    char pressMpxm[8];		// PPP.PP	(kPa)
    char pressMS5607[8];	//
    char humHIH6030[8];		//
    char extHumidity[8];		// RRR.RR	(%RH)
    char extTemp275[8];		// +TTT.T Ext Dig. temperature (C)
    char extTempRTD[8];		// +TTT.T RTD temperature (C)
    vector3Sample_t acc;	// Accelerometer subsample (m/s^2)
    vector3Sample_t mag;	// Magnetometer subsample (g)
}dataSample_t;

void datasample_init(dataSample_t *s);
int8_t datasample_writeToLog(dataSample_t *s, logfile_t *log);
void datasample_gpsToSample(gpsLocation_t *loc, dataSample_t *samp);


#endif