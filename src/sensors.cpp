#include "sensors.h"             // Public declaration of this task.
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht.h"
#include "esp_adc/adc_oneshot.h"
#include "sensor_data.h"  // Measurement format.
#include "rtos_objects.h" // Shared queue.

// Label for messages from the sensor module.
static const char *TAG = "ROOM_MONITOR";

// Read temperature, humidity, and light approximately every two seconds.
void sensorTask(void *parameter)
{
     // Create an ADC1 instance, owned and used by this task.
    adc_oneshot_unit_handle_t adcHandle = nullptr;
    adc_oneshot_unit_init_cfg_t unitConfig = {};
    unitConfig.unit_id = ADC_UNIT_1;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unitConfig, &adcHandle));

     // GPIO 34 is ADC1 channel 6 on this ESP32.
    adc_oneshot_chan_cfg_t channelConfig = {};
    channelConfig.bitwidth = ADC_BITWIDTH_12;
    channelConfig.atten = ADC_ATTEN_DB_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adcHandle, ADC_CHANNEL_6, &channelConfig));

    // Let the sensor settle after power-up.
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    
    // Reference time for the repeating two-second schedule.
    TickType_t lastWakeTime = xTaskGetTickCount();    
    
    while (true) {
        float temperature = 0;
        float humidity = 0;

        // DHT22 uses AM2301 mode; its data wire connects to GPIO 4.
        esp_err_t result = dht_read_float_data(
            DHT_TYPE_AM2301, GPIO_NUM_4, &humidity, &temperature);

        // Publish temperature independently of the light measurement.
        if (result == ESP_OK) {
            xQueueOverwrite(alarmQueue, &temperature);
        }

        // Read the LDR before taking the output mutex.
        int lightRaw = 0;
        esp_err_t lightResult = adc_oneshot_read(
            adcHandle, ADC_CHANNEL_6, &lightRaw);

        // Both reads are finished. Lock only while printing the report.
        if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
            if (result == ESP_OK) {
                ESP_LOGI(TAG, "Temperature: %.2f C | Humidity: %.2f %%",
                         temperature, humidity);
            } else {
                ESP_LOGW(TAG, "DHT read failed: %s",
                         esp_err_to_name(result));
            }

            if (lightResult == ESP_OK) {
                // Relative brightness, not calibrated lux.
                int lightPercent = ((4095 - lightRaw) * 100) / 4095;
                ESP_LOGI(TAG, "Light: %d%% | Raw: %d",
                         lightPercent, lightRaw);
            } else {
                ESP_LOGW(TAG, "LDR read failed: %s",
                         esp_err_to_name(lightResult));
            }

            // Let another task print.
            xSemaphoreGive(serialMutex);
        }
        
        // Send only when both sensor reads succeeded.
        if (result == ESP_OK && lightResult == ESP_OK) {
            SensorData readings{};
            readings.temperature = temperature;
            readings.humidity = humidity;
            readings.lightLevel = ((4095 - lightRaw) * 100) / 4095;
            // Motion is communicated separately through systemEvents.

            // Copy the readings into the queue without waiting.
            if (xQueueSend(sensorQueue, &readings, 0) != pdTRUE) {
                if (xSemaphoreTake(serialMutex, portMAX_DELAY) == pdTRUE) {
                    ESP_LOGW(TAG, "Sensor queue full; reading dropped");
                    xSemaphoreGive(serialMutex);
                }
            }
        }
        
        // Wait until the next scheduled sampling time to reduce drift.
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}



