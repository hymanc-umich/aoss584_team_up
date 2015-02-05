/**
 * gps.c
 * NMEA-0183 GPS Interface
 * AOSS 584, Winter 2015
 * University of Michigan
 * Author: Cody Hyman
 */

#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include "chbsem.h"

//#define DEBUG_GPS
#define DEBUG_SERIAL &SD2

/**
 * @brief GPS Receiver interface struct
 */
struct gpsReceiver
{
   UARTDriver *gpsUart; 		// UART Driver
   char buffer[2][GPS_BUFFER_SIZE]; 	// Double receive buffer
   uint8_t bufferCount;			// Receiver buffer counter
   uint8_t activeBuffer; 		// Double buffer switch
   gpsLocation_t location;		// Location storage
   mutex_t dataMutex;			// Data mutex
};
typedef struct gpsReceiver gpsReceiver_t;

typedef struct gpsThread gpsThread_t;

static gpsReceiver_t GPS;

#ifdef DEBUG_GPS
    static SerialDriver *DEBUG = DEBUG_SERIAL;
#endif

binary_semaphore_t gpsSem;

/** GPS UART Callbacks **/

/**
 * @brief Double buffer selector
 * @return Active receive buffer
 */
static char * gpsGetActiveRxBuffer(void)
{
    return GPS.buffer[GPS.activeBuffer];
}

/**
 * @brief Add character to GPS buffer
 * @param c Character to add
 * @return Success 
 */
static int8_t gpsAddToBuffer(char c)
{
    if(GPS.bufferCount >= GPS_BUFFER_SIZE)
	return -1;
    char * buffer = gpsGetActiveRxBuffer();
    buffer[GPS.bufferCount++] = c;
    return 0;
}

/**
 * @brief Toggles the active GPS receive buffer
 */
static inline void gpsToggleBuffer(void)
{
    GPS.activeBuffer ^= 1; 	// Toggle double buffer
    GPS.bufferCount = 0; 	// Reset buffer count
}

/**
 * @brief GPS Tx buffer empty
 */
static void gpsTxbufEnd(UARTDriver *uartp)
{
   (void) uartp;// TODO: Nothing? 
}

/**
 * @brief GPS Tx complete
 */
static void gpsTxC(UARTDriver *uartp)
{
    (void) uartp;
}

/**
 * @brief GPS Rx character received callback
 */
static void gpsRxChar(UARTDriver *uartp, uint16_t c)
{
    (void) uartp; // TODO: Check character and place in buffer
    if(c == '\n')
    {
	gpsAddToBuffer('\0');
	chBSemSignal(&gpsSem);
    }
    else
    {
	gpsAddToBuffer(c);
    }
}

/**
 * @brief GPS RxBuffer full callback
 */
static void gpsRxEnd(UARTDriver *uartp)
{
    (void) uartp;// Add to buffer
}

/**
 * @brief GPS UART Rx Error callback
 */
static void gpsRxErr(UARTDriver *uartp, uartflags_t e)
{
    (void) uartp;
    (void) e;
}


/*
 * UART driver configuration structure.
 */
static UARTConfig serGpsCfg = {
  gpsTxbufEnd,
  gpsTxC,
  gpsRxEnd,
  gpsRxChar,
  gpsRxErr,
  NMEA_0183_BAUD,
  0,
  0,	// No LIN
  0
};


/**
 * @brief GPS interface startup
 * @param gpsUART GPS UART driver
 * @return Initialization success status
 */
int8_t gpsStart(UARTDriver *gpsUart)
{
    if(gpsUart == NULL)
	return -1;
    GPS.gpsUart = gpsUart;
    chMtxObjectInit(&(GPS.dataMutex));
    uartStart(GPS.gpsUart, &serGpsCfg); // Start GPS Serial port
    return 0;
}

/**
 * @brief Stops GPS receiver
 * @param gps GPS receiver to stop
 * @return Stop success flag
 */
int8_t gpsStop(gpsReceiver_t *gps)
{
    uartStop(gps->gpsUart);
    return 0; 
}

/**
 * @brief Parses NMEA 
 * @param nmeaStr NMEA Sentence string to parse
 * @return NMEA GPS Sentence type
 */
sentenceType_t gpsParseNMEAType(char *nmeaStr)
{
    char *stypeIndex;
    if(nmeaStr[0] == NMEA_START_CHAR)
    {
	if(strncmp(nmeaStr+1, NMEA_GPS, 2) == 0)
	{
	    stypeIndex = nmeaStr + 3;
	    if(strncmp(stypeIndex, NMEA_GPS_ALM, 3) == 0)
	    {
		return GPS_ALM;
	    }
	    else if(strncmp(stypeIndex, NMEA_FIX_DATA, 3) == 0)
	    {
		return FIX_DATA;
	    }
	    else if(strncmp(stypeIndex, NMEA_GEO_POS,3) == 0)
	    {
		return GEO_POS;
	    }
	    else if(strncmp(stypeIndex, NMEA_ACT_SAT, 3) == 0)
	    {
		return ACT_SAT;
	    }
	    else if(strncmp(stypeIndex, NMEA_SAT_VIEW, 3) == 0)
	    {
		return SAT_VIEW;
	    }
	    else if(strncmp(stypeIndex, NMEA_HEADING, 3) == 0)
	    {
		return HEADING;
	    }
	    else if(strncmp(stypeIndex, NMEA_DATE_TIME, 3) == 0)
	    {
		return DATE_TIME;
	    }
	    else
	    {
		return OTHER;
	    }
	}
	else
	{
	    return OTHER;
	}
    }
    else
	return UNDEF;
}

/**
 * @brief Parse GPS GGA Sentence for location
 * @param nmeaGGAstr NMEA GPS Fix ($GPGGA) string
 * @param loc Location to store to
 * @return Parse success status
 */

int8_t gpsParseFix(char *nmeaGGAStr, gpsLocation_t *loc)
{
    // Verify GGA Sentence
    if(strncmp(nmeaGGAStr+3, NMEA_FIX_DATA, 3)) 
	return -1;
    // Parse sentence
    uint8_t i = 0;
    char tokBuf[17];
    uint8_t pos = 7;
    uint8_t bufferCount;
    for(i = 1; i<10; i++)
    {
	bufferCount = 0;
	while((nmeaGGAStr[pos] != ',') 
	    && (nmeaGGAStr[pos] != '\0') 
	    && (bufferCount < 16))
	{
	    tokBuf[bufferCount++] = nmeaGGAStr[pos++];
	}
	tokBuf[bufferCount] = '\0';
	pos++;
	switch(i)
	{
	    case 1: // Time
		// HHMMSS.SS
		if(strlen(tokBuf) > 0))
		{
		    loc->time[0] = tokBuf[0]; // H1
		    loc->time[1] = tokBuf[1]; // H2
		    loc->time[2] = ':';
		    loc->time[3] = tokBuf[2]; // M1
		    loc->time[4] = tokBuf[3]; // M2
		    loc->time[5] = ':';
		    loc->time[6] = tokBuf[4]; // S1
		    loc->time[7] = tokBuf[5]; // S2
		    loc->time[8] = '.';
		    loc->time[9] = tokBuf[7];  // MS1
		    loc->time[10] = tokBuf[8]; // MS2
		    loc->time[11] = '\0';
		}
		else
		    loc->time[0] = '\0';
		strcpy(loc->time,tokBuf);
		break;
	    case 2: // Latitude;
		strcpy(loc->latitude, tokBuf);
		break;
	    case 3: // Latitude Hemisphere
		strcat(loc->latitude, tokBuf);
		break;
	    case 4: // Longitude
		strcpy(loc->longitude, tokBuf);
		break;
	    case 5: // Longitude Hemisphere
		strcat(loc->longitude, tokBuf);
		break;
	    case 6: // Quality Indicator (Do nothing for now)
		break;
	    case 7: // Satellite count
		strcpy(loc->satCount, tokBuf);
		break;
	    case 8: // HDOP
		break;
	    case 9: // Altitude
		strcpy(loc->altitude, tokBuf);
		break;
	    default:
		break;
	}
    }
    return 0;
}

/**
 * @brief Computes the NMEA-0183 sentence checksum
 * @param nmeaStr String to parse checksum
 */
uint8_t gpsNMEAChecksum(char *nmeaStr)
{
    uint8_t checksum = 0;
    nmeaStr++; // Skip initial character
    while(*nmeaStr)
	checksum ^= *(nmeaStr++);
    return checksum;
}

/**
 * @brief GPS Data Collection Thread
 * @param arg GPS thread structure pointer
 */
msg_t gpsThread(void *arg)
{
    gpsThread_t *thread = (gpsThread_t *) arg;
    msg_t message;
    chBSemObjectInit(&gpsSem, false);
    // GPS Loop
    while(thread->running)
    {
	chBSemWait(&gpsSem); 		// Wait for new NMEA string
	chBSemReset(&gpsSem, false);	// Clear semaphore
	// Parse Fix data if sentence is correct
	if(gpsParseNMEAType(gpsGetActiveRxBuffer()) == FIX_DATA)
	{
	    chMtxLock(&(GPS.mutex));
	    gpsParseFix(gpsGetActiveRxBuffer(), &(GPS.location));
	    chMtxUnlock(&(GPS.mutex));
	}
    }
    gpsStop(&GPS);
    return message;
}

/**
 * @brief Thread safe accessor for GPS latitude
 * @param dest GPS return latitude DDMMM.MMMMMN
 */
void gpsGetLatitude(char *dest)
{
    chMtxLock((&GPS.dataMutex));
    strcpy(dest, GPS.location.latitude);
    chMtxUnlock((&GPS.dataMutex));
}

/**
 * @brief Thread safe accessor for GPS longitude
 * @param GPS return longitude DDDMMM.MMMMMW
 */
void gpsGetLongitude(char *dest)
{
    chMtxLock(&(GPS.dataMutex));
    strcpy(dest, GPS.location.longitude);
    chMtxUnlock(&(GPS.dataMutex));
}

/**
 * @brief Thread safe accessor for GPS altitude
 * @param dest GPS return altitude in receiver units, typ. meters
 */
void gpsGetAltitude(char *dest)
{
    chMtxLock(&(GPS.dataMutex));
    strcpy(dest, GPS.location.altitude);
    chMtxUnlock(&(GPS.dataMutex));
}

/**
 * @brief Thread safe accessor for GPS time
 * @param dest GPS return time (hh:mm:ss.ss)
 */
void gpsGetTime(char *dest)
{
    chMtxLock(&(GPS.dataMutex));
    strcpy(dest, GPS.location.time);
    chMtxUnlock(&(GPS.dataMutex));
}

/**
 * @brief Thread safe accessor for GPS satellite count
 * @param dest Number of visible GPS satellites return
 */
void gpsGetSatellites(char *dest)
{
    chMtxLock(&(GPS.dataMutex));
    strcpy(dest, GPS.location.satCount);
    chMtxUnlock(&(GPS.dataMutex));
}

/**
 * @brief Thread safe accessor to copy all location data
 * @param dest Destination location container
 */
void gpsGetLocation(gpsLocation_t *dest)
{
    chMtxLock(&(GPS.dataMutex));
    strcpy(dest->latitude  , GPS.location.latitude);
    strcpy(dest->longitude , GPS.location.longitude);
    strcpy(dest->altitude  , GPS.location.altitude);
    strcpy(dest->time      , GPS.location.time);
    strcpy(dest->satCount  , GPS.location.satCount);
    chMtxUnlock(&(GPS.dataMutex));
}
