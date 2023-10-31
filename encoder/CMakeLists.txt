# add_executable(encoder
#         encoder.c
#         )

# # pull in common dependencies and additional pwm hardware support
# target_link_libraries(encoder pico_stdlib hardware_pwm)

# # create map/bin/hex file etc.
# pico_add_extra_outputs(encoder)

# pico_enable_stdio_usb(encoder 1)
# # add url via pico_set_program_url
# example_auto_set_url(encoder)

add_library(pico_encoder encoder.h encoder.c)

target_link_libraries(pico_encoder pico_stdlib hardware_gpio hardware_timer)

target_include_directories(pico_encoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
