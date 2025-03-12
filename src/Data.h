#ifndef POOLMASTER_DATA_H
#define POOLMASTER_DATA_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "Config.h"
#include "PoolMaster.h"

#define NUMBER_OF_HISTORY_SAMPLES   360

// Used for tasks
void stack_mon(UBaseType_t&);

// RTOS Task
void StoreHistory(void *);

#endif