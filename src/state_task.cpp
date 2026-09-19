#include "system_state.h"
#include "rtos_objects.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "STATE";

void stateTask(void *parameter)
{
    // Allow the full timeout after startup before becoming inactive.
    TickType_t lastMotionTick = xTaskGetTickCount();
    TickType_t lastWakeTick = lastMotionTick;
    SystemState previousState = SystemState::ACTIVE;

    while (true) {
        TickType_t now = xTaskGetTickCount();

        // Inspect the motion flag without clearing it.
        bool motionDetected =
            (xEventGroupGetBits(systemEvents) & EVENT_MOTION) != 0;

        // Keep refreshing the timer while the PIR reports motion.
        if (motionDetected) {
            lastMotionTick = now;
        }

        // Unsigned tick subtraction handles the tick counter wrapping.
        uint32_t elapsedMs =
            static_cast<uint32_t>(now - lastMotionTick)
            * portTICK_PERIOD_MS;

        SystemState state =
            determineSystemState(motionDetected, elapsedMs);

        // Publish and log only when the state changes.
        if (state != previousState) {
            if (state == SystemState::INACTIVE) {
                xEventGroupSetBits(systemEvents, EVENT_INACTIVE);
                ESP_LOGI(TAG, "INACTIVE");
            } else {
                xEventGroupClearBits(systemEvents, EVENT_INACTIVE);
                ESP_LOGI(TAG, "ACTIVE");
            }

            previousState = state;
        }

        // Check every 50 ms, allowing other tasks to run between checks.
        vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(50));
    }
}