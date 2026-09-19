#include "alarm.h"

// Laboratory temperature limits, in degrees Celsius.
static constexpr float LOW_LIMIT = 18.0f;
static constexpr float HIGH_LIMIT = 30.0f;

// Pure decision logic: no GPIO, buzzer, or FreeRTOS calls.
AlarmState evaluateTemperature(float temperature)
{
    if (temperature < LOW_LIMIT) {
        return AlarmState::LOW_TEMPERATURE;
    }

    if (temperature > HIGH_LIMIT) {
        return AlarmState::HIGH_TEMPERATURE;
    }

    return AlarmState::NORMAL;
}