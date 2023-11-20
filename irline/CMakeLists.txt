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

target_link_libraries(pico_irline 
                      pico_stdlib
                      hardware_adc
                      FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
                      
                      )

target_include_directories(pico_irline PUBLIC 
                            ${CMAKE_CURRENT_LIST_DIR}
                            ${CMAKE_CURRENT_LIST_DIR}/..
                            )

#example __________________________
# add_executable(wifi
#         wifi.c
#         )
# target_compile_definitions(wifi PRIVATE
#         WIFI_SSID=\"${WIFI_SSID}\"
#         WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
#         )
# target_include_directories(wifi PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}
#         ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
#         )
# target_link_libraries(wifi
#         pico_cyw43_arch_lwip_threadsafe_background
#         pico_stdlib
#         pico_lwip_iperf
#         FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
#         pico_irline
#         )

# pico_enable_stdio_usb(wifi 1)
# pico_add_extra_outputs(wifi)