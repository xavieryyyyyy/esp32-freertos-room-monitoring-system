#include "display_mode.h"

// Forward navigation, including wraparound.
DisplayMode nextDisplayMode(DisplayMode current)
{
    switch (current) {
        case DisplayMode::TEMPERATURE: return DisplayMode::HUMIDITY;
        case DisplayMode::HUMIDITY:    return DisplayMode::LIGHT;
        case DisplayMode::LIGHT:       return DisplayMode::MOTION;
        case DisplayMode::MOTION:      return DisplayMode::TEMPERATURE;
    }

    // Recover to a known page if given an invalid value.
    return DisplayMode::TEMPERATURE;
}

// Reverse navigation, including wraparound.
DisplayMode previousDisplayMode(DisplayMode current)
{
    switch (current) {
        case DisplayMode::TEMPERATURE: return DisplayMode::MOTION;
        case DisplayMode::HUMIDITY:    return DisplayMode::TEMPERATURE;
        case DisplayMode::LIGHT:       return DisplayMode::HUMIDITY;
        case DisplayMode::MOTION:      return DisplayMode::LIGHT;
    }

    return DisplayMode::TEMPERATURE;
}