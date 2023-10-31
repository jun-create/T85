add_executable(cmExample cmExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(cmExample pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(cmExample 1)
pico_enable_stdio_uart(cmExample 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(cmExample)

# add url via pico_set_program_url
example_auto_set_url(cmExample)

#
add_executable(irlineExample irlineExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(irlineExample pico_stdlib hardware_uart pico_irline)

pico_enable_stdio_usb(irlineExample 1)
pico_enable_stdio_uart(irlineExample 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(irlineExample)

# add url via pico_set_program_url
example_auto_set_url(irlineExample)

#
add_executable(magnometerExample
        magnometerExample.c
        )

# pull in common dependencies and additional i2c hardware support
target_link_libraries(magnometerExample pico_stdlib hardware_i2c pico_magnometer)

# create map/bin/hex file etc.
pico_add_extra_outputs(magnometerExample)

# add url via pico_set_program_url
example_auto_set_url(magnometerExample)


pico_enable_stdio_usb(magnometerExample 1)

