#pragma once

// Possible results of checking the temperature.
enum class AlarmState {
    NORMAL,
    LOW_TEMPERATURE,
    HIGH_TEMPERATURE
};

// Decide the alarm state without accessing hardware.
AlarmState evaluateTemperature(float temperature);

// FreeRTOS task that evaluates readings and controls the buzzer.
void alarmTask(void *parameter);