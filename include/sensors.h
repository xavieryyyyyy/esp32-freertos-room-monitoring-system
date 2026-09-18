#pragma once

// FreeRTOS task that reads temperature, humidity, and relative light.
// The task input parameter is currently unused.
void sensorTask(void *parameter);