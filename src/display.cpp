#include "display.h"      // Declaration of displayTask().
#include "sensor_data.h"  // Format of received measurements.
#include "rtos_objects.h" // Shared sensor queue.
#include "esp_log.h"
#include "ssd1306.h"      // OLED driver.
#include "esp_err.h"      // Initialization error checks.
#include <cstdio>         // snprintf: format a number as text.
#include <cstring>        // strlen: count the text characters.
#include "display_mode.h" // Named display pages.

// Identify messages from the display module.
static const char *TAG = "DISPLAY";

// Own the OLED and display the selected measurement.
void displayTask(void *parameter)
{
    // Only DisplayTask accesses this OLED.
    SSD1306_t oled{};

    // SDA = GPIO 21, SCL = GPIO 22; -1 means no reset pin.
    i2c_master_init(&oled, 21, 22, -1);
    ESP_ERROR_CHECK(ssd1306_init(&oled, 128, 64));

    // Clear the screen and write 12 characters on text row 0.
    ssd1306_clear_screen(&oled, false);
    ssd1306_display_text(&oled, 0, "ROOM MONITOR", 12, false);

    SensorData readings{};

    // Remember the selected page and whether sensor data has arrived.
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    bool haveReadings = false;
       
    // Track motion separately from queued environmental measurements.
    bool motionDetected = false;

    // Remember whether the OLED is currently powered off.
    bool displayInactive = false;
    
    while (true)
    {
        bool redraw = false;

        // Read the activity state published by StateTask.
        bool inactive =
        (xEventGroupGetBits(systemEvents) & EVENT_INACTIVE) != 0;

        // Send a power command only when the state changes.
        if (inactive != displayInactive) {
        displayInactive = inactive;

        if (inactive) {
        ssd1306_display_off(&oled);
        ESP_LOGI(TAG, "OLED off");
        } else {
        ssd1306_display_on(&oled);
        redraw = true; // Refresh the selected page after waking.
        ESP_LOGI(TAG, "OLED on");
    }
}
        
        // Read the motion bit without clearing it.
        bool latestMotion =
            (xEventGroupGetBits(systemEvents) & EVENT_MOTION) != 0;

        if (latestMotion != motionDetected) {
            motionDetected = latestMotion;
            if (currentMode == DisplayMode::MOTION) {
                redraw = true;
            }
        }

        // Wait briefly for measurements so page changes stay responsive.
        if (xQueueReceive(sensorQueue, &readings,
                          pdMS_TO_TICKS(50)) == pdTRUE)
        {
            haveReadings = true;
            redraw = true;
        }

        // Check for a page selection without waiting.
        if (xQueueReceive(displayModeQueue, &currentMode, 0) == pdTRUE)
        {
            redraw = true;
        }

        // Draw only while active and when the selected page has data.
        if (!displayInactive && redraw &&
        (currentMode == DisplayMode::MOTION || haveReadings))
        {
            const char *label = "";
            char text[17] = {};

            switch (currentMode)
            {
            case DisplayMode::TEMPERATURE:
                label = "Temperature";
                snprintf(text, sizeof(text), "%.1f C", readings.temperature);
                break;
            case DisplayMode::HUMIDITY:
                label = "Humidity";
                snprintf(text, sizeof(text), "%.1f %%", readings.humidity);
                break;
            case DisplayMode::LIGHT:
                label = "Light";
                snprintf(text, sizeof(text), "%d %%", readings.lightLevel);
                break;
            case DisplayMode::MOTION:
                label = "Motion";
                snprintf(text, sizeof(text), "%s",
                         motionDetected ? "Detected" : "No motion");
                break;
            }

            // Clear old text so shorter labels leave no extra characters.
            ssd1306_clear_line(&oled, 2, false);
            ssd1306_clear_line(&oled, 4, false);
            ssd1306_display_text(&oled, 2, label, strlen(label), false);
            ssd1306_display_text(&oled, 4, text, strlen(text), false);
            ESP_LOGI(TAG, "%s: %s", label, text);
        }
    } // End of repeating loop.
} // End of displayTask.
