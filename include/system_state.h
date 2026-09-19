#pragma once

#include <cstdint>

// Whether the system's display should be awake or inactive.
enum class SystemState {
    ACTIVE,
    INACTIVE
};

// Enter inactivity after 15 seconds without motion.
constexpr uint32_t INACTIVITY_TIMEOUT_MS = 15000;

// Decide the state without accessing hardware.
// elapsedWithoutMotionMs is the time since motion was last present.
SystemState determineSystemState(
    bool motionDetected,
    uint32_t elapsedWithoutMotionMs
);

// Track time without motion and publish the system's activity state.
void stateTask(void *parameter);