#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ROOM_MONITOR";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "BCA152 FreeRTOS Multisensor");
    ESP_LOGI(TAG, "System starting...");

    while (true) {
        ESP_LOGI(TAG, "System running - serial output OK");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
