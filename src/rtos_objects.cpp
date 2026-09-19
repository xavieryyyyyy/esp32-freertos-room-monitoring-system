#include "rtos_objects.h"

// Define the shared handle once.
// nullptr means no queue has been created yet.
QueueHandle_t sensorQueue = nullptr;

// Created during startup before either task uses it.
QueueHandle_t displayModeQueue = nullptr;

// Created during startup before sensor and alarm tasks begin.
QueueHandle_t alarmQueue = nullptr;