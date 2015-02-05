#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* NMEA Sentence Definitions */
// NMEA Start Character
#define NMEA_START_CHAR		'$'
#define NMEA_START 		"$"
// NMEA Talker Identifiers
#define NMEA_GPS		"GP"
// NMEA Sentence Identifiers
#define NMEA_GPS_ALM		"ALM"
#define NMEA_FIX_DATA		"GGA"
#define NMEA_GEO_POS		"GLL"
#define NMEA_ACT_SAT		"GSA"
#define NMEA_SAT_VIEW		"GSV"
#define NMEA_HEADING		"HDT"
#define NMEA_DATE_TIME		"ZDA"

typedef enum
{
    GPS_ALM,
    FIX_DATA,
    GEO_POS,
    ACT_SAT,
    SAT_VIEW,
    HEADING,
    DATE_TIME,
    OTHER,
    UNDEF
}sentenceType_t;

typedef struct
{
    int64_t latitude;
    int64_t longitude;
    int32_t altitude;
    uint32_t time;
    uint8_t satCount;
}gpsLocation_t;

typedef struct
{
    char latitude[14]; 	// DDMM.MMMMMH
    char longitude[15]; // DDDMMM.MMMMMH
    char altitude[8];	// AAAA.AA [m]
    char time[10];	// HHMMSS.SS
    char satCount[3];	// QQ
}gpsLocationStr_t;

void printLocation(gpsLocation_t *loc)
{
    printf("Lat:%ld,Lon:%ld,Alt:%d,Time:%d,Sat:%d\n", 
	   loc->latitude,
	   loc->longitude,
	   loc->altitude,
	   loc->time,
	   loc->satCount);
}

void printLocationStr(gpsLocationStr_t *loc)
{
    printf("Lat:%s   Lon:%s   Alt:%sm   Time:%s   Sat:%s\n", 
	   loc->latitude,
	   loc->longitude,
	   loc->altitude,
	   loc->time,
	   loc->satCount);
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

int8_t gpsParseFixStr(char *nmeaGGAStr, gpsLocationStr_t *loc)
{
    // Verify GGA Sentence
    if(strncmp(nmeaGGAStr+3, NMEA_FIX_DATA, 3)) 
    {
	printf("Sentence type incorrect\n");
	return -1;
    }
    
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
		// HHMMSS
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
}
    
int8_t gpsParseFix(char *nmeaGGAStr, gpsLocation_t *loc)
{
    // Verify GGA Sentence
    if(strncmp(nmeaGGAStr+3, NMEA_FIX_DATA, 3)) 
	return -1;
    
    // Parse sentence
    //char * itok = strtok(nmeaGGAStr,",");
    uint8_t i = 0;
    char tokBuf[16];
    uint8_t pos = 7;
    uint8_t bufferCount;
    for(i = 1; i<10; i++)
    {
	bufferCount = 0;
	while((nmeaGGAStr[pos] != ',') 
	    && (nmeaGGAStr[pos] != '\0') 
	    && (bufferCount < 16))
	{
	    if(nmeaGGAStr[pos] != '.')
		tokBuf[bufferCount++] = nmeaGGAStr[pos++];
	    else
		pos++; // Skip periods
	}
	tokBuf[bufferCount] = '\0';
	pos++;
	switch(i)
	{
	    case 1: // Time
		// HHMMSS
		printf("tokBufTIME:%s\n",tokBuf);
		loc->time = atoi(tokBuf);
		break;
	    case 2: // Latitude
		printf("tokBufLAT:%s\n",tokBuf);
		loc->latitude = atol(tokBuf);
		break;
	    case 3: // Latitude Hemisphere
		printf("tokBufLatHemi:%s\n",tokBuf);
		if(tokBuf[0] == 'S')
		    loc->latitude = -1 * loc->latitude;
		break;
	    case 4: // Longitude
		printf("tokBufLONG:%s\n",tokBuf);
		loc->longitude = atol(tokBuf);
		break;
	    case 5: // Longitude Hemisphere
		printf("tokBufLongHemi:%s\n",tokBuf);
		if(tokBuf[0] == 'W')
		    loc->longitude = -1 * loc->latitude;
		break;
	    case 6: // Quality Indicator (Do nothing for now)
		printf("tokBufQUAL:%s\n",tokBuf);
		break;
	    case 7: // Satellite count
		printf("tokBufSAT:%s\n",tokBuf);
		loc->satCount = atoi(tokBuf);
		break;
	    case 8: // HDOP
		printf("tokBufHDOP:%s\n",tokBuf);
		break;
	    case 9: // Altitude
		printf("tokBufALT:%s\n", tokBuf);
		loc->altitude = atoi(tokBuf);
		break;
	    default:
		break;
	}
    }
}

/**
 * @brief Computes the NMEA-0183 sentence checksum
 */
uint8_t gpsNMEAChecksum(char *nmeaStr)
{
    uint8_t checksum = 0;
    nmeaStr++; // Skip initial character
    while(*nmeaStr)
	checksum ^= *nmeaStr++;
    return checksum;
}


int main(void)
{
    gpsLocationStr_t location;
    char *buffer= "$GPGLL,001038.00,3334.2313457,S,11211.0576940,W,2,04,5.4,354.682,M,-26.574,M,7.0,0138*79";
    
    int bufLen = strlen(buffer);
    printf("Sentence size:%d\n", bufLen);
    gpsParseFixStr(buffer, &location);
    printLocationStr(&location);
    return 0;
}
