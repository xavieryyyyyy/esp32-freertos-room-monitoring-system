#include "rtos_objects.h"

// Define the shared handle once.
// nullptr means no queue has been created yet.
QueueHandle_t sensorQueue = nullptr;