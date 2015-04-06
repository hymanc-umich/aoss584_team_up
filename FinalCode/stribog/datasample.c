#include "datasample.h"
#include "chprintf.h"

/**
 * @brief Initializes the master data sample
 */
void datasample_init(dataSample_t *s)
{
    char *emptyStr = "-";
    // TODO: Update write to sample
    uStrCpy(s->time, "-:-:-");
    uStrCpy(s->gps.latitude, emptyStr);
    uStrCpy(s->gps.longitude, emptyStr);
    uStrCpy(s->gps.altitude, emptyStr);
    uStrCpy(s->gps.satCount, emptyStr);
    uStrCpy(s->tempBMP, emptyStr);
    uStrCpy(s->extTemp275, emptyStr);
    uStrCpy(s->extTempRTD, emptyStr);
    uStrCpy(s->pressBMP, emptyStr);
    uStrCpy(s->pressMpxm, emptyStr);
    uStrCpy(s->pressMS5607, emptyStr);
    uStrCpy(s->intHumidity, emptyStr);
    uStrCpy(s->extHumidity, emptyStr);
    uStrCpy(s->humHIH6030, emptyStr);
    uStrCpy(s->acc.x, emptyStr);
    uStrCpy(s->acc.y, emptyStr);
    uStrCpy(s->acc.z, emptyStr);
    uStrCpy(s->mag.x, emptyStr);
    uStrCpy(s->mag.y, emptyStr);
    uStrCpy(s->mag.z, emptyStr);
}

/**
 * @brief Writes a master sample to the logfile
 * @param log Logfile to write to
 * @param ms Master sample
 * @return Status of writing sample to log
 */
static char dataPrintBuf[384];
int8_t datasample_writeToLog(dataSample_t *s, logfile_t *log)
{
    s->sn++; // Increment sample counter
    //TIME,LAT,LONG,ALT,SAT,TMPI,TMPE1,TMPE2,PRESSI,PRESSE1,PRESSE2,HUMDI,HUMDE1,HUMDE2,ACCX,ACCY,ACCZ,MAGX,MAGY,MAGZ
    int len = chsnprintf(dataPrintBuf, 256, "%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
			 s->sn,
             (char *)(s->time),
			 (char *)(s->gps.latitude),
			 (char *)(s->gps.longitude),
			 (char *)(s->gps.altitude),
			 (char *)(s->gps.satCount),
			 (char *)(s->tempBMP),
			 (char *)(s->extTemp275),
			 (char *)(s->extTempRTD),
			 (char *)(s->pressBMP),
			 (char *)(s->pressMpxm),
			 (char *)(s->pressMS5607),
			 (char *)(s->intHumidity),
			 (char *)(s->extHumidity),
			 (char *)(s->humHIH6030),
			 (char *)(s->acc.x),
			 (char *)(s->acc.y),
			 (char *)(s->acc.z),
			 (char *)(s->mag.x),
			 (char *)(s->mag.y),
			 (char *)(s->mag.z)
    );  
    return logfileWrite(log, dataPrintBuf, len, false);
}

/**
 * @brief Copies GPS data into a data sample for writing
 * @param loc GPS Location
 * @param samp Data sample to store to
 */
void datasample_gpsToSample(gpsLocation_t *loc, dataSample_t *samp)
{
    uStrCpy(samp->time, loc->time); // Copy
    uStrCpy(samp->gps.latitude, loc->latitude);
    uStrCpy(samp->gps.longitude, loc->longitude);
    uStrCpy(samp->gps.altitude, loc->altitude);
    uStrCpy(samp->gps.satCount, loc->satCount);
}

/**
 * @brief Print a data sample to the terminal
 * @param samp Data sample
 */
void printDataSample(dataSample_t *samp)
{
   chprintf((BaseSequentialStream *) &DBG_SERIAL, "TIM:%s, LAT:%s, LNG:%s, ALT:%s, SAT:%s, TMPI:%s, TMPE:%s, PRS:%s, HUMI:%s, HUME:%s, ACC(%s,%s,%s)\n",
	    (char *)(samp->time),
	    (char *)(samp->gps.latitude),
	    (char *)(samp->gps.longitude),
	    (char *)(samp->gps.altitude),
	    (char *)(samp->gps.satCount),
	    (char *)(samp->tempBMP),
	    (char *)(samp->extTemp275),
	    (char *)(samp->pressMpxm),
	    (char *)(samp->intHumidity),
	    (char *)(samp->extHumidity),
	    (char *)(samp->acc.x),
	    (char *)(samp->acc.y),
	    (char *)(samp->acc.z)
   );
}