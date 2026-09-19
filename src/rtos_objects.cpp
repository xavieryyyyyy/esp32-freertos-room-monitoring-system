#include "rtos_objects.h"

// Define the shared handle once.
// nullptr means no queue has been created yet.
QueueHandle_t sensorQueue = nullptr;

// Created during startup before either task uses it.
QueueHandle_t displayModeQueue = nullptr;

// Created during startup before sensor and alarm tasks begin.
QueueHandle_t alarmQueue = nullptr;

// Created during startup; shared by tasks that publish or read status.
EventGroupHandle_t systemEvents = nullptr;

// Created in app_main before any task prints protected reports.
SemaphoreHandle_t serialMutex = nullptr;