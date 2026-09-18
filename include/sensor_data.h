#pragma once

// One set of measurements to share between tasks.
struct SensorData {
    float temperature;
    float humidity;
    int lightLevel;       // Relative light percentage: 0–100.
    bool motionDetected;  // Motion support will be added later.
};