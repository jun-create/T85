# add_executable(wifi
#         wifi.c
#         )
add_library(wifi wifi.h wifi.c)
target_compile_definitions(wifi PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(wifi PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(wifi
        pico_cyw43_arch_lwip_threadsafe_background
        # pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

# pico_enable_stdio_usb(wifi 1)
# pico_add_extra_outputs(wifi)
