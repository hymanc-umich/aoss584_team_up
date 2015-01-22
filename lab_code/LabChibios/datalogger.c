#include "datalogger.h"

/**
 * @brief Datalogger instance
 */
/*
struct datalogger
{
   sdmmc_t *sdc; 	// SD structure
   FATFS *filesys;	// Filesystem
   uint16_t nfiles; 	// Number of logfiles open
   char *logPath; 	// Logfile path
   bool driveMounted;	// Drive mounted
};
*/

/**
 * @brief Logfile instance 
 */
/*
struct logfile
{
    uint16_t id;		// Instance ID
    char *name; 		// Instance name
    datalogger_t *logger; 	// Logger parent
    FIL *file;			// File pointer
};
*/

/**
 * @brief Subclassed separated value log
 */
/*
struct separatedLog
{
    uint16_t id;	// Instance ID
    char separator;	// Separator character
    uint16_t ncols;	// Number of columns
    logfile_t logfile;	// Parent items
};
*/

/**
 * @brief Initializes a Data Logger instance
 * @param logger
 * @param logPath Path to store logs to
 * @param sd SD Card container
 * @return Success state of data logger initialization
 */
bool dataLoggerInitialize(datalogger_t *logger, char *logPath, sdmmc_t *sd)
{
    if(sd == NULL || logger == NULL)
	return false;
    if(logPath == NULL)
	logPath = "0:";
    FRESULT err;
    //disk_initialize(0);
    logger->sdc = sd;
    logger->filesys = sdmmcGetFS(sd);
    logger->logPath = logPath;
    err = f_mount(logger->filesys, logPath, 0);
    if(err != FR_OK)
    {
	return false;
    }
    logger->driveMounted = true;
    return true;
}

/**
 * @brief Stops an ongoing data logging process
 */
bool dataLoggerStop(datalogger_t *logger)
{
    // TODO: Close all files
    f_mount(NULL, NULL, 0);
    logger->driveMounted = false;
    return false;
}

/**
 * @brief Creates a new logfile instance
 */
int8_t logfileNew(logfile_t *log, datalogger_t *logger)
{
    FRESULT res;
    if(!logger->driveMounted)
	return false;
    res = f_open(log->file, "0:testfile.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if(res)
    {
	return res;
    }
    return 0;
}

/**
 * @brief Checks the size of a logfile
 */
size_t logfileSize(logfile_t *log)
{
    // TODO: Get file size
    return 0;
}

/**
 * @brief Writes a buffer to a logfile
 */
int8_t logfileWrite(logfile_t *log, char *buf, uint16_t length)
{
    FRESULT res;
    int written;
    res = fwrite(log->file, buf, length, &written);
    if(res || (written != length))
	return 1;
    return 0;
}

/**
 * @brief Writes a separated value list to a logfile
 * @param log Logfile instance to write to
 * @param items String items
 * @param separator Separator character
 * @param nitems Total number of items to write
 */
int8_t logfileWriteCsv(logfile_t *log, char **items, char separator, uint16_t nitems)
{
    FRESULT res;
    // Format CSV line
    char * buf;
    uint32_t length;
    int written;
    uint16_t i, j;
    for(i = 0; i < nitems; i++)
    {
	j = 0;
	while(items[i][j] != '\0')
	{
	    buf[length++] = items[i][j++];
	}
	buf[length++] = separator;
    }
    // TODO: Write CSV line to buffer
    //res = fwrite(log->file, bufLen, &written);
    //if(res || (written != length))
	//return 1;
    return 0;
}

/**
 * @brief Closes a logfile
 * @param log Logfile to close
 * @return Success status
 */
int8_t logfileClose(logfile_t *log)
{
    FRESULT res;
    if(f_close(log->file))
	return 1;
    return 0;
}