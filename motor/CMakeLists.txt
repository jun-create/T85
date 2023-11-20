# example _________________________________________________________________
# add_executable(motor
#         motor.c
#         )

# # pull in common dependencies and additional pwm hardware support
# target_link_libraries(motor 
                        # pico_stdlib
#                         hardware_pwm
#                         pico_encoder
#                         )

# # create map/bin/hex file etc.
# pico_add_extra_outputs(motor)

# pico_enable_stdio_usb(motor 1)
# # add url via pico_set_program_url
# example_auto_set_url(motor)

# example _________________________________________________________________
add_library(pico_motor motor.h motor.c)

target_link_libraries(pico_motor 
                        pico_stdlib
                        hardware_pwm
                        pico_encoder
                        )

target_include_directories(pico_motor PUBLIC 
                            ${CMAKE_CURRENT_LIST_DIR}
                            ${CMAKE_CURRENT_LIST_DIR}/..
                            )
