add_library(irline irline.h irline.c)

# pull in common dependencies and additional pwm hardware support
target_link_libraries(irline pico_stdlib hardware_adc FreeRTOS-Kernel-Heap4)
target_link_libraries(irline motor server)
pico_enable_stdio_usb(irline 1)

target_include_directories(irline PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"/..)
