#include "datalogger.h"

#include "chprintf.h"

static SerialDriver *DEBUG; // Debug

/**
 * @brief Initializes a Data Logger instance
 * @param logger
 * @param logPath Path to store logs to
 * @param sd SD Card container
 * @param dbg Debug Serial Port
 * @return Success state of data logger initialization
 */
int8_t dataLoggerInitialize(datalogger_t *logger, char *logPath, sdmmc_t *sd, SerialDriver *dbg)
{
    if(sd == NULL || logger == NULL)
	return false;
    DEBUG = dbg;
    if(logPath == NULL)
	logPath = "";
    FRESULT err;
    logger->sdc = sd;
    logger->filesys = sdmmcGetFS(sd);
    logger->logPath = logPath;
    err = f_mount(logger->filesys, logPath, 1);
    if(err != FR_OK)
    {
	chprintf((BaseSequentialStream *) DEBUG, "DATALOGGER: Error Mounting Filesystem,ERR%20d\n",err);
	return -1;
    }
    logger->driveMounted = true;
    chprintf((BaseSequentialStream *) DEBUG, "DATALOGGER: Datalogger Started\n");
    return 0;
}

/**
 * @brief Stops an ongoing data logging process
 */
bool dataLoggerStop(datalogger_t *logger)
{
    // TODO: Close all files
    chprintf((BaseSequentialStream *) DEBUG, "DATALOGGER: Datalogger Stopped\n");
    f_mount(NULL, NULL, 0);
    logger->driveMounted = false;
    return false;
}

/**
 * @brief Creates a new logfile instance
 */
int8_t logfileNew(logfile_t *log, datalogger_t *logger, FIL *file, char *fname)
{
    chprintf((BaseSequentialStream *) DEBUG, "DATALOGGER: Creating New Logfile\n");
    FRESULT res;
    if(!(logger->driveMounted))
	return false;
    log->file = file;
    log->wrCount = 0;
    log->name = fname;
    res = f_open(log->file, fname, FA_CREATE_ALWAYS | FA_WRITE);
    if(res)
    {
	chprintf((BaseSequentialStream *) DEBUG, "DATALOGGER: Error creating Logfile, ERR%02d\n",res);
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
int8_t logfileWrite(logfile_t *log, char *buf, uint16_t length, bool openClose)
{
    FRESULT res;
    int written;
    if(openClose)
    {
	res = logfileOpenAppend(log->file);
	if(res)
	    return res;
    }
    res = f_write(log->file, buf, length, &written);
    if(res || (written != length))
	return 1;
    log->wrCount++;
    if(openClose)
    {
	res = f_close(log->file);
	if(res)
	    return res;
    }
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
    //res = f_write(log->file, bufLen, &written);
    //if(res || (written != length))
	//return 1;
    return 0;
}

/**
 * @brief Accessor for logfile write count
 */
uint32_t logfileGetWrCount(logfile_t *log)
{
    return log->wrCount;
}

/**
 * @brief Closes a logfile
 * @param log Logfile to close
 * @return Success status
 */
int8_t logfileClose(logfile_t *log)
{
    if(f_close(log->file))
    {
	log->wrCount = 0;
	return 1;
    }
    return 0;
}

/**
 * @brief Opens a logfile and seeks to the end for appending
 * @param log Logfile container
 * @return Status of successful open
 */
int8_t logfileOpenAppend(logfile_t *log)
{
    FRESULT res;
    res = f_open(log->file, log->name, FA_WRITE | FA_OPEN_ALWAYS);
    if(res == FR_OK)
    {
	res = f_lseek(log->file, f_size(log->file));
	if(res != FR_OK)
	    f_close(log->file);
    }
    return res;
}

/**
 * @brief Data Logger Thread
 * @param arg Data thread structure pointer
 */
msg_t dataloggerThread(void *arg)
{
    dataThread_t *thread = (dataThread_t *) arg;
    while(thread->running)
    {
	// Write data to storage
	chThdSleepMilliseconds(thread->sleepTime); // Sleep
    }
}