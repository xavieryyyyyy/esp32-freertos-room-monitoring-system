#include "input.h"        // InputTask declaration.
#include "display_mode.h" // Page navigation logic.
#include "rtos_objects.h" // Queue for the selected page.
#include "driver/gpio.h"  // ESP-IDF GPIO functions.
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

// Identify encoder-related messages.
static const char *TAG = "INPUT";

// Match the encoder connections in diagram.json.
static constexpr gpio_num_t ENCODER_CLK = GPIO_NUM_18;
static constexpr gpio_num_t ENCODER_DT = GPIO_NUM_19;

// Private queue: carries rotation directions from the interrupt to InputTask.
static QueueHandle_t encoderQueue = nullptr;

// Called on a falling edge of CLK: HIGH on DT means clockwise.
static void encoderInterrupt(void *argument)
{
    const bool clockwise = gpio_get_level(ENCODER_DT) != 0;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    // Interrupt handlers must use the FromISR version of queue operations.
    xQueueSendFromISR(
        encoderQueue, &clockwise, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// Process encoder events and publish the latest selected page.
void inputTask(void *parameter)
{
    // Create the event queue before enabling encoder interrupts.
    encoderQueue = xQueueCreate(16, sizeof(bool));
    configASSERT(encoderQueue != nullptr);

    DisplayMode currentMode = DisplayMode::TEMPERATURE;

    // Publish the initial page.
    xQueueOverwrite(displayModeQueue, &currentMode);

    // Configure CLK and DT as inputs with pull-ups.
    gpio_config_t config = {};
    config.pin_bit_mask =
        (1ULL << ENCODER_CLK) | (1ULL << ENCODER_DT);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    // Install the shared GPIO interrupt service.
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Trigger the handler when CLK changes from HIGH to LOW.
    ESP_ERROR_CHECK(gpio_set_intr_type(ENCODER_CLK, GPIO_INTR_NEGEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(
        ENCODER_CLK, encoderInterrupt, nullptr));

        
    bool clockwise = false;
    while (true) {
        // Wait for a rotation event without continuously using CPU time.
        if (xQueueReceive(encoderQueue, &clockwise, portMAX_DELAY) == pdTRUE) {
            currentMode = clockwise
                ? nextDisplayMode(currentMode)
                : previousDisplayMode(currentMode);

            // Publish the latest selected page.
            xQueueOverwrite(displayModeQueue, &currentMode);
            ESP_LOGI(TAG, "Encoder: %s",
                     clockwise ? "clockwise" : "counterclockwise");
        }
    }
}

