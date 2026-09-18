cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-freertos-room-monitoring-system)

# Give the DHT driver access to GPIO headers and functions on newer ESP-IDF.
idf_component_get_property(dht_lib esp-idf-lib__dht COMPONENT_LIB)
target_link_libraries(${dht_lib} PUBLIC idf::esp_driver_gpio)