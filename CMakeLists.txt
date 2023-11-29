
cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH "C:\\Program Files\\Raspberry Pi\\Pico SDK v1.5.1\\pico-sdk")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "I want compile_commands.json" FORCE)
# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

include(FreeRTOS_Kernel_import.cmake)

project(INF2004_LAB1-main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_executable(taskmanager
        taskmanager.c
        )

if (NOT PICO_NO_HARDWARE)
    add_subdirectory(distance)
    add_subdirectory(irline)
    add_subdirectory(magnometer)
    add_subdirectory(motor)
    add_subdirectory(wifi)
    # add_subdirectory(main)
endif ()

# pull in common dependencies
target_link_libraries(taskmanager pico_stdlib hardware_pwm hardware_adc)
target_link_libraries(taskmanager wifi irline pico_ultrasonic)
pico_enable_stdio_usb(taskmanager 1)
pico_enable_stdio_uart(taskmanager 0)


# # create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(taskmanager)
