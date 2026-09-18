#pragma once

// Named pages that the rotary encoder will select.
enum class DisplayMode {
    TEMPERATURE,
    HUMIDITY,
    LIGHT,
    MOTION
};

// Select the next page, wrapping from Motion to Temperature.
DisplayMode nextDisplayMode(DisplayMode current);

// Select the previous page, wrapping from Temperature to Motion.
DisplayMode previousDisplayMode(DisplayMode current);

