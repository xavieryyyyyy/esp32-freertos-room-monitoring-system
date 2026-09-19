# Compile our application source file.
# ESP-IDF reads idf_component.yml separately for dependencies.
idf_component_register(
    SRCS "sensors.cpp" "main.cpp" "rtos_objects.cpp" "display.cpp" "display_mode.cpp" "input.cpp" "alarm_logic.cpp" "alarm.cpp"
    INCLUDE_DIRS "." "../include"
    PRIV_REQUIRES esp_adc esp_driver_i2c esp_driver_spi esp_driver_gpio esp_driver_ledc
)