#____________________________
add_executable(taskmanager
        taskmanager.c
        )
target_compile_definitions(taskmanager PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(taskmanager PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
# pull in common dependencies and additional i2c hardware support
target_link_libraries(taskmanager 
                      pico_stdlib
                      hardware_i2c
                      
                      pico_encoder
                      pico_irline
                      pico_magnometer
                      pico_motor
                      pico_ultrasonic
                      pico_wifi
                        
                      pico_cyw43_arch_lwip_threadsafe_background
                      pico_lwip_iperf # for wifi
                      FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
                      )

# create map/bin/hex file etc.
pico_add_extra_outputs(taskmanager)

# add url via pico_set_program_url
example_auto_set_url(taskmanager)


pico_enable_stdio_usb(taskmanager 1)


#____________________________
add_executable(cmExample cmExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(cmExample pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(cmExample 1)
pico_enable_stdio_uart(cmExample 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(cmExample)

# add url via pico_set_program_url
example_auto_set_url(cmExample)

#____________________________
add_executable(irlineExample irlineExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(irlineExample pico_stdlib hardware_uart pico_irline)

pico_enable_stdio_usb(irlineExample 1)
pico_enable_stdio_uart(irlineExample 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(irlineExample)

# add url via pico_set_program_url
example_auto_set_url(irlineExample)

#____________________________
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

