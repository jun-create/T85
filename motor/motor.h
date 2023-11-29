#include "stdint.h"
#ifndef motor_h
#define motor_h
void init_engine();
void init_motor(uint16_t default_speed);
void forward();
void backwards();
void stop();
void left_tilt();
void right_tilt();
void set_speed(uint16_t current_speed);
// void move_forward_with_distance(int wheel_encoder_pin, int IN1_PIN, int IN2_PIN, int IN3_PIN, int IN4_PIN, double distance);

#define right_wheel_encoder_pin 3
#define left_wheel_encoder_pin 2
extern volatile long long g_left_wheel_code;
extern volatile long long g_right_wheel_code;
void left_wheel_encoder_handler();
void right_wheel_encoder_handler();
void rotate_clockwise();
void rotate_counter_clockwise();
void reset_wheel_encoder();
#define DIST_5CM 10
#define DIST_10CM 20
#define DIST_20CM 40
#define DIST_100CM 200

#endif