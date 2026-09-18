#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Shared queue handle; storage will be defined in rtos_objects.cpp.
extern QueueHandle_t sensorQueue;

// Carries the selected page from InputTask to DisplayTask.
extern QueueHandle_t displayModeQueue;