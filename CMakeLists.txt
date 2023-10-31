if (NOT PICO_NO_HARDWARE)
    include(FreeRTOS_Kernel_import.cmake)

    add_subdirectory(encoder)
    add_subdirectory(Examples)
    add_subdirectory(irline)
    add_subdirectory(magnometer)
    add_subdirectory(motor)
    add_subdirectory(ultrasonic)
    add_subdirectory(wifi)

endif ()
