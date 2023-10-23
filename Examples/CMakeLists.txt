add_executable(centimeter cmExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(centimeter pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(centimeter 1)
pico_enable_stdio_uart(centimeter 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(centimeter)

# add url via pico_set_program_url
example_auto_set_url(centimeter)
