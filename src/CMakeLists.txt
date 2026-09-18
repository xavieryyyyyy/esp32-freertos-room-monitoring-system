# Compile our application source file.
# ESP-IDF reads idf_component.yml separately for dependencies.
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
)