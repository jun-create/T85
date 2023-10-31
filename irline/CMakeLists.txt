# add_executable(irline
#         irline.c
#         )

# # pull in common dependencies and additional pwm hardware support
# target_link_libraries(irline pico_stdlib hardware_adc)
# pico_enable_stdio_usb(irline 1)

# # create map/bin/hex file etc.
# pico_add_extra_outputs(irline)

# # add url via pico_set_program_url
# example_auto_set_url(irline)

add_library(pico_irline irline.h irline.c)

target_link_libraries(pico_irline pico_stdlib hardware_adc)

target_include_directories(pico_irline PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")