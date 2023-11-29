add_library(magnometer magnometer.h magnometer.c)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(magnometer pico_stdlib hardware_i2c)
target_include_directories(magnometer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

pico_enable_stdio_usb(magnometer 1)