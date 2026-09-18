#include "esp_log.h"             // ESP-IDF logging, including ESP_LOGI().
#include "freertos/FreeRTOS.h"    // FreeRTOS definitions and time conversion macros.
#include "freertos/task.h"        // Task creation and blocking-delay functions.
#include "sensors.h" // Declares the sensor task implemented in sensors.cpp.
#include "sensor_data.h"  // Format of each queued reading.
#include "rtos_objects.h" // Shared queue handle.
#include "display.h" // Declares the display task.
#include "display_mode.h" // Type of page-selection messages.
#include "input.h" // Rotary-encoder task.


// Label attached to our log messages so we can identify their source.
static const char *TAG = "ROOM_MONITOR";

// ESP-IDF entry point, already running inside a FreeRTOS task.
// extern "C" lets ESP-IDF find this function by its C name in our C++ file.
extern "C" void app_main(void)
{
    
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");
    
    // Store up to five complete SensorData items.
    sensorQueue = xQueueCreate(5, sizeof(SensorData));
    configASSERT(sensorQueue != nullptr);

    // Keep only the latest requested display page.
    displayModeQueue = xQueueCreate(1, sizeof(DisplayMode));
    configASSERT(displayModeQueue != nullptr);
    
    // Start SensorTask: 2048-byte stack, no input, priority 1, no saved handle.
    BaseType_t resultA = xTaskCreate(
        sensorTask, "SensorTask", 2048, nullptr, 1, nullptr);
    configASSERT(resultA == pdPASS);

      // Start DisplayTask as the sole receiver of sensorQueue.
    BaseType_t displayResult = xTaskCreate(
        displayTask, "DisplayTask", 4096, nullptr, 1, nullptr);
    configASSERT(displayResult == pdPASS);

    // Process user input promptly; block when no encoder events arrive.
    BaseType_t inputResult = xTaskCreate(
        inputTask, "InputTask", 3072, nullptr, 3, nullptr);
    configASSERT(inputResult == pdPASS);
}

