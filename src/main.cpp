#include "esp_log.h"             // ESP-IDF logging, including ESP_LOGI().
#include "freertos/FreeRTOS.h"    // FreeRTOS definitions and time conversion macros.
#include "freertos/task.h"        // Task creation and blocking-delay functions.
#include "dht.h"       // DHT sensor reading functions.
#include "esp_err.h"   // Converts error codes into readable names.
#include "esp_adc/adc_oneshot.h" // Read individual analog measurements.

// Label attached to our log messages so we can identify their source.
static const char *TAG = "ROOM_MONITOR";

// Read temperature, humidity, and light approximately every two seconds.
static void taskA(void *parameter)
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

    while (true) {
        float temperature = 0;
        float humidity = 0;

        // DHT22 uses AM2301 mode; its data wire connects to GPIO 4.
        esp_err_t result = dht_read_float_data(
            DHT_TYPE_AM2301, GPIO_NUM_4, &humidity, &temperature);

        if (result == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.2f C | Humidity: %.2f %%",
                     temperature, humidity);
        } else {
            ESP_LOGW(TAG, "DHT read failed: %s",
                     esp_err_to_name(result));
        }

        // Read the LDR voltage as a raw 12-bit ADC value.
        int lightRaw = 0;
        esp_err_t lightResult = adc_oneshot_read(
            adcHandle, ADC_CHANNEL_6, &lightRaw);

        if (lightResult == ESP_OK) {
            // Invert the ADC scale: lower voltage means brighter light.
            // Relative indication only; this is not calibrated lux.
            int lightPercent = ((4095 - lightRaw) * 100) / 4095;

            ESP_LOGI(TAG, "Light: %d%% | Raw: %d", lightPercent, lightRaw);
        } else {
            ESP_LOGW(TAG, "LDR read failed: %s",
                     esp_err_to_name(lightResult));
        }

        // Wait at least two seconds before reading again.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Practice task B: print approximately every two seconds once created.
static void taskB(void *parameter)
{
    while (true) {
        ESP_LOGI(TAG, "Task B running");
       // Block for two seconds so other ready tasks can run.
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
