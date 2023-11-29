/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "motor.h"
#include "magnometer.h"

// Left Motor
#define ENB_PIN 5
#define IN3_PIN 6
#define IN4_PIN 7

// Right Motor
#define ENA_PIN 20
#define IN1_PIN 19
#define IN2_PIN 18

//Wheel values
#define CIRCUMFERENCE 21
#define NUM_OF_HOLES 20
#define LEFT_WHEEL_PIN 0xC0 //bit 6 and 7
#define LW_FW 0x40
#define LW_RV 0x80

#define RIGHT_WHEEL_PIN 0xC0000 //bit 18 and 19
#define RW_RV 0x40000 //bit 18
#define RW_FW 0x80000 //bit 19


volatile long long  g_left_wheel_code = 0;
volatile long long  g_right_wheel_code = 0;
volatile unsigned int speed = 0;


uint slice_num_1;
uint slice_num_2;

//Initialise the respective gpio pins
void init_engine() {

    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_init(IN3_PIN);
    gpio_init(IN4_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_set_dir(IN3_PIN, GPIO_OUT);
    gpio_set_dir(IN4_PIN, GPIO_OUT);
}

//initialise the motor
void init_motor(uint16_t default_speed) {

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENB_PIN, GPIO_FUNC_PWM);

    slice_num_1 = pwm_gpio_to_slice_num(ENA_PIN);
    slice_num_2 = pwm_gpio_to_slice_num(ENB_PIN);

    pwm_set_clkdiv(slice_num_1, 100);
    pwm_set_clkdiv(slice_num_2, 100);

    pwm_set_wrap(slice_num_1, default_speed);
    pwm_set_wrap(slice_num_2, default_speed);

    pwm_set_chan_level(slice_num_1, PWM_CHAN_A, default_speed);
    pwm_set_chan_level(slice_num_2, PWM_CHAN_B, default_speed);

    pwm_set_enabled(slice_num_1, true);
    pwm_set_enabled(slice_num_2, true);
}

//Move forward
void forward() {
    gpio_clr_mask(RW_RV | LW_RV);
    gpio_set_mask(RW_FW | LW_FW);
}

//Move backward
void backwards() {
    gpio_clr_mask(RW_FW | LW_FW);
    gpio_set_mask(RW_RV | LW_RV);
}

void rotate_clockwise(){
    gpio_clr_mask(LEFT_WHEEL_PIN | RIGHT_WHEEL_PIN);
    gpio_set_mask(RW_RV | LW_FW);
}

void rotate_counter_clockwise(){
    gpio_clr_mask(LEFT_WHEEL_PIN | RIGHT_WHEEL_PIN);
    gpio_set_mask(RW_FW | LW_RV);
}

void stop() {
    gpio_clr_mask(LEFT_WHEEL_PIN | RIGHT_WHEEL_PIN);
}

bool is_stop() {
    uint32_t pins = gpio_get_all();
    return 0 == (pins & (LEFT_WHEEL_PIN | RIGHT_WHEEL_PIN));
}

void stop_left() {
    gpio_clr_mask(LEFT_WHEEL_PIN);
}

void stop_right() {
    gpio_clr_mask(RIGHT_WHEEL_PIN);
}

void set_speed(uint16_t current_speed){
    speed = current_speed;
    pwm_set_chan_level(slice_num_1, PWM_CHAN_A, current_speed);
    pwm_set_chan_level(slice_num_2, PWM_CHAN_B, current_speed);
    return;
}

//Turn left 
void left_tilt() {
    //Slow right motor
    pwm_set_chan_level(slice_num_2, PWM_CHAN_B, speed * 0.5);
}

//Turn right
void right_tilt() {
    //Slow left motor
    pwm_set_chan_level(slice_num_1, PWM_CHAN_A, speed * 0.8);
}

void left_wheel_encoder_handler(uint32_t events){
    ++g_left_wheel_code;
}

void right_wheel_encoder_handler(uint32_t events){
    ++g_right_wheel_code;
}

void reset_wheel_encoder(){
    g_left_wheel_code = 0;
    g_right_wheel_code = 0;
}
