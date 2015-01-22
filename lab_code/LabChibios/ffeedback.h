#ifndef FFEEDBACK_H
#define FFEEDBACK_H

#include "ch.h"
#include "hal.h"

#define MIN_FEEDBACK
#define MAX_FEEDBACK

typedef struct
{
    // TODO: ChibiOS Mutex
    uint16_t force_value;
    uint16_t setpoint;
}force_feedback_t;

#endif