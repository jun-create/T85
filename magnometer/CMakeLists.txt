# add_executable(magnometer
#         magnometer.c
#         )

# # pull in common dependencies and additional i2c hardware support
# target_link_libraries(magnometer pico_stdlib hardware_i2c)

# # create map/bin/hex file etc.
# pico_add_extra_outputs(magnometer)

# # add url via pico_set_program_url
# example_auto_set_url(magnometer)


# pico_enable_stdio_usb(magnometer 1)



add_library(pico_magnometer magnometer.h magnometer.c)

target_link_libraries(pico_magnometer pico_stdlib hardware_i2c)

target_include_directories(pico_magnometer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")