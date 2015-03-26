#include "sensor_thread.h"

/**
 * 
 */
msg_t sensorThread(void *arg)
{
    sensorThread_t *thread = (sensorThread_t *) arg;
    msg_t message;
    chMtxObjectInit(&thread->data.mtx); // Initialize Mutex
    // Sensor Loop
    while(thread->running)
    {
	chThdSleepMilliseconds(20);
    }

    return message;
}

/**
 * 
 */
msg_t sensorThreadStop(sensorThread_t *thread)
{
    thread->running = false;
}

/**
 * @brief Safely acquires sensor data
 */
int8_t getSensorData(sensorThread_t *thread, sensorData_t *buffer)
{
    chMtxLock(&thread->data.mtx);     // Check Mutex
    // Copy data over to buffer
    chMtxUnlock(&thread->data.mtx);
    return -1;
}