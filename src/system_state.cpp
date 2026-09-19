#include "system_state.h"

SystemState determineSystemState(
    bool motionDetected,
    uint32_t elapsedWithoutMotionMs)
{
    // Motion keeps the system awake or wakes it immediately.
    if (motionDetected) {
        return SystemState::ACTIVE;
    }

    // No motion for the full timeout means the system is inactive.
    if (elapsedWithoutMotionMs >= INACTIVITY_TIMEOUT_MS) {
        return SystemState::INACTIVE;
    }

    // Stay awake while waiting for the timeout.
    return SystemState::ACTIVE;
}