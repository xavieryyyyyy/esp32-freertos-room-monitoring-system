#include "alarm.h"        // Alarm decision logic and task declaration.
#include "rtos_objects.h" // Temperature queue.
#include "esp_log.h"
#include "driver/ledc.h" // Hardware-generated tone signal.
#include "esp_err.h"    // Check driver configuration errors.

static const char *TAG = "ALARM";

// Evaluate queued temperatures and control the buzzer.
void alarmTask(void *parameter)
{
    // Configure a 2000 Hz tone timer with 10-bit duty resolution.
    ledc_timer_config_t timerConfig = {};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.freq_hz = 2000;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    // Route the tone timer to GPIO 25, starting silently.
    ledc_channel_config_t channelConfig = {};
    channelConfig.gpio_num = 25;
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = LEDC_CHANNEL_0;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;
    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));

    float temperature = 0;

    while (true) {
        // Block until a valid temperature arrives.
        if (xQueueReceive(alarmQueue, &temperature, portMAX_DELAY) == pdTRUE) {
            AlarmState state = evaluateTemperature(temperature);

            // Normal: silent. Low or high temperature: 50% duty tone.
            uint32_t duty = (state == AlarmState::NORMAL) ? 0 : 512;

            ESP_ERROR_CHECK(ledc_set_duty(
                LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
            ESP_ERROR_CHECK(ledc_update_duty(
                LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

           // Wait for other tasks to finish their diagnostic report.
           if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
            
        switch (state) {
            case AlarmState::NORMAL:
                ESP_LOGI(TAG, "NORMAL: %.2f C", temperature);
                break;

            case AlarmState::LOW_TEMPERATURE:
                ESP_LOGW(TAG, "LOW temperature: %.2f C", temperature);
                break;

            case AlarmState::HIGH_TEMPERATURE:
                ESP_LOGW(TAG, "HIGH temperature: %.2f C", temperature);
                break;
    }

    // Release the output resource after printing.
    xSemaphoreGive(serialMutex);
}
        }
    }
}