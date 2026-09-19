#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

// Shared queue handle; storage will be defined in rtos_objects.cpp.
extern QueueHandle_t sensorQueue;

// Carries the selected page from InputTask to DisplayTask.
extern QueueHandle_t displayModeQueue;

// Delivers temperature readings to AlarmTask independently of DisplayTask.
extern QueueHandle_t alarmQueue;

// Shared status flags; MotionTask will maintain the motion bit.
extern EventGroupHandle_t systemEvents;

// Set while the PIR output indicates motion.
constexpr EventBits_t EVENT_MOTION = BIT0;

// Set when the inactivity timeout expires; cleared while active.
constexpr EventBits_t EVENT_INACTIVE = BIT1;