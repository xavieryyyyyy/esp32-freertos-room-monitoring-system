#include "esp_log.h"             // ESP-IDF logging, including ESP_LOGI().
#include "freertos/FreeRTOS.h"    // FreeRTOS definitions and time conversion macros.
#include "freertos/task.h"        // Task creation and blocking-delay functions.
#include "sensors.h" // Declares the sensor task implemented in sensors.cpp.
#include "sensor_data.h"  // Format of each queued reading.
#include "rtos_objects.h" // Shared queue handle.

// Label attached to our log messages so we can identify their source.
static const char *TAG = "ROOM_MONITOR";


// Temporary diagnostic consumer: wait for a reading, then print it.
static void taskB(void *parameter)
{
    SensorData readings{};

    while (true) {
        // Block until an item arrives; receiving removes it from the queue.
        if (xQueueReceive(sensorQueue, &readings, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG,
                     "Queue received: %.2f C | %.2f %% | Light: %d%%",
                     readings.temperature,
                     readings.humidity,
                     readings.lightLevel);
        }
    }
}

// ESP-IDF entry point, already running inside a FreeRTOS task.
// extern "C" lets ESP-IDF find this function by its C name in our C++ file.
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");
    
    // Store up to five complete SensorData items.
    sensorQueue = xQueueCreate(5, sizeof(SensorData));
    configASSERT(sensorQueue != nullptr);
    
    // Start SensorTask: 2048-byte stack, no input, priority 1, no saved handle.
    BaseType_t resultA = xTaskCreate(
        sensorTask, "SensorTask", 2048, nullptr, 1, nullptr);
    configASSERT(resultA == pdPASS);

    // Start Task B at the same priority and check that creation succeeded.
    BaseType_t resultB = xTaskCreate(
        taskB, "TaskB", 2048, nullptr, 1, nullptr);
    configASSERT(resultB == pdPASS);
}

