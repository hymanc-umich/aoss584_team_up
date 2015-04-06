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

    s->bmp280Temp = 0;
    s->bmp280Press = 0;
    s->vin = 0;
    s->i7020Humd = 0;
    s->i7020Temp = 0;
    s->e7020Humd = 0;
    s->e7020Temp = 0;
    s->ms5607Temp = 0;
    s->ms5607Press = 0;
    s->mpxmPress = 0;
    s->hih6030Humd = 0;
    s->hih6030Temp = 0;
    s->tmp275Temp = 0;
    s->rtdTemp = 0;
    s->acc[0] = 0;
    s->acc[1] = 0;
    s->acc[2] = 0;
    s->mag[0] = 0;
    s->mag[1] = 0;
    s->mag[2] = 0;
}

/**
 * @brief Writes a master sample to the logfile
 * @param log Logfile to write to
 * @param ms Master sample
 * @return Status of writing sample to log
 */
static char dataPrintBuf[768];
int8_t datasample_writeToLog(dataSample_t *s, logfile_t *log)
{
	s->sn++; // Increment sample counter
  //"SN,GPS_TIME,GPS_LAT,GPS_LONG,GPS_ALT,GPS_SAT,TEMP_TMP275,TEMP_SI7020I,HUM_SI7020I,TEMP_BMP280,PRESS_BMP280,TEMP_SI7020E,HUM_SI7020E,TEMP_HIH6030,HUM_HIH6030,TEMP_MS5607,PRESS_MS5607,PRESS_MPXM,TEMP_RTD,VIN,AX,AY,AZ,MX,MY,MZ\n"

	int len = chsnprintf(dataPrintBuf, 256, "%d,%s,%s,%s,%s,%.1f,%.2f,%.2f,%.2f,%.2f,%.1f,%.2f,%.2f,%.2f,%.3f,%.3f,%.2f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
						s->sn,
						(char *)(s->time),
						(char *)(s->gps.latitude),
			 			(char *)(s->gps.longitude),
			 			(char *)(s->gps.altitude),
			 			(char *)(s->gps.satCount),
			 			s->tmp275Temp,
			 			s->i7020Temp, s->i7020Humd,
			 			s->bmp280Temp, s->bmp280Press,
			 			s->e7020Temp, s->e7020Humd,
			 			s->hih6030Temp, s->hih6030Humd,
			 			s->ms5607Temp, s->ms5607Press,
			 			s->mpxmPress, s->rtdTemp,
			 			s->vin,
			 			s->acc[0],s->acc[1],s->acc[2],
			 			s->mag[0],s->mag[1],s->mag[2]
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
 * @brief Copies sensor data to data sample for writing
 * @param sensors Sensor data collection (sensor_thread sensorData_t)
 * @param samp Data sample to record to
 */
void datasample_sensorsToSample(sensorData_t *sensors, dataSample_t *samp)
{
	samp->bmp280Temp = sensors->tempBmp;
	samp->bmp280Press = sensors->pressBmp;
	samp->i7020Temp = sensors->temp7020Int;
	samp->i7020Humd = sensors->humd7020Int;
	samp->e7020Temp = sensors->temp7020Ext;
	samp->e7020Humd = sensors->humd7020Ext;
	samp->ms5607Temp = sensors->tempMs5607;
	samp->ms5607Press = sensors->pressMs5607;
	samp->mpxmPress = sensors->pressMpxm;
	samp->hih6030Temp = sensors->temp6030;
	samp->hih6030Humd = sensors->humd6030;
	samp->tmp275Temp = sensors->temp275;
	samp->rtdTemp = sensors->tempRtd;
	samp->vin = sensors->vin;
	samp->acc[0] = sensors->accX;
	samp->acc[1] = sensors->accY;
	samp->acc[2] = sensors->accZ;
	samp->mag[0] = sensors->magX;
	samp->mag[1] = sensors->magY;
	samp->mag[2] = sensors->magZ;
}
/**
 * @brief Print a data sample snippet to the terminal (not full sample)
 * @param samp Data sample
 */
 /*
void printDataSample(dataSample_t *samp)
{
   chprintf((BaseSequentialStream *) &DBG_SERIAL, "TIM:%s, LAT:%s, LNG:%s, ALT:%s, SAT:%s, TMPI:%s, TMPE:%s, PRS:%s, HUMI:%s, HUME:%s, ACC(%s,%s,%s)\n",
		(char *)(samp->time),
		(char *)(samp->gps.latitude),
		(char *)(samp->gps.longitude),
		(char *)(samp->gps.altitude),
		(char *)(samp->gps.satCount),
		(char *)(samp->i7020Temp),
		(char *)(samp->extTemp275),
		(char *)(samp->pressMpxm),
		(char *)(samp->intHumidity),
		(char *)(samp->extHumidity),
		(char *)(samp->acc.x),
		(char *)(samp->acc.y),
		(char *)(samp->acc.z)
   );
}
*/