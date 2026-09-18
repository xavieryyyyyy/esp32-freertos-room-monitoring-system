#include "esp_log.h"             // ESP-IDF logging, including ESP_LOGI().
#include "freertos/FreeRTOS.h"    // FreeRTOS definitions and time conversion macros.
#include "freertos/task.h"        // Task creation and blocking-delay functions.

// Label attached to our log messages so we can identify their source.
static const char *TAG = "ROOM_MONITOR";

// Practice task A: print approximately every second once this task is created.
// FreeRTOS requires a void* input parameter; this task does not use it.
static void taskA(void *parameter)
{
    while (true) {
        ESP_LOGI(TAG, "Task A running");
        // Convert milliseconds to scheduler ticks and block so other tasks can run.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Practice task B: print approximately every two seconds once created.
static void taskB(void *parameter)
{
    while (true) {
        ESP_LOGI(TAG, "Task B running");
        // Block for two seconds to make this task's timing different from task A.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ESP-IDF entry point, already running inside a FreeRTOS task.
// extern "C" lets ESP-IDF find this function by its C name in our C++ file.
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");

    // Start Task A: 2048-byte stack, no input, priority 1, no saved handle.
    BaseType_t resultA = xTaskCreate(
        taskA, "TaskA", 2048, nullptr, 1, nullptr);
    configASSERT(resultA == pdPASS);

    // Start Task B at the same priority and check that creation succeeded.
    BaseType_t resultB = xTaskCreate(
        taskB, "TaskB", 2048, nullptr, 1, nullptr);
    configASSERT(resultB == pdPASS);
}
