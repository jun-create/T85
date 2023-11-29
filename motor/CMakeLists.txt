add_library(motor motor.h motor.c)
# pull in common dependencies and additional pwm hardware support
target_link_libraries(motor pico_stdlib hardware_gpio hardware_timer hardware_pwm)
target_link_libraries(motor magnometer)
target_include_directories(motor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
