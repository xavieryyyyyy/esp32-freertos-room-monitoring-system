#include "motion.h"
#include "driver/gpio.h" // Read the PIR output.
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtos_objects.h" // Shared event group and motion bit.

static const char *TAG = "MOTION";
static constexpr gpio_num_t PIR_PIN = GPIO_NUM_27;

// Monitor the PIR and log its initial level and subsequent changes.
void motionTask(void *parameter)
{
    gpio_config_t config = {};
    config.pin_bit_mask = 1ULL << PIR_PIN;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    int previousLevel = -1;

    while (true) {
        int level = gpio_get_level(PIR_PIN);

        if (level != previousLevel) {
            
            // Publish the PIR level for other tasks to inspect.
            if (level != 0) {
                xEventGroupSetBits(systemEvents, EVENT_MOTION);
            } else {
                xEventGroupClearBits(systemEvents, EVENT_MOTION);
            }
            
            ESP_LOGI(TAG, "%s", level ? "Motion detected" : "No motion");
            previousLevel = level;
        }

        // Check every 50 ms, blocking between checks.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}