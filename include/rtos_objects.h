#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Shared queue handle; storage will be defined in rtos_objects.cpp.
extern QueueHandle_t sensorQueue;