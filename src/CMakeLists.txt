# Compile our application source file.
# ESP-IDF reads idf_component.yml separately for dependencies.
idf_component_register(
    SRCS "sensors.cpp" "main.cpp" "rtos_objects.cpp"
    INCLUDE_DIRS "." "../include"
    PRIV_REQUIRES esp_adc
)