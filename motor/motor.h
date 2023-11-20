#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdio.h>

// Motor control pins
#define ENA_PIN 10
#define IN1_PIN 11
#define IN2_PIN 12

#define ENB_PIN 13
#define IN3_PIN 14
#define IN4_PIN 15

// Wheel circumference
#define CIRCUMFERENCE 21

// Turn time in milliseconds
#define MOVE_TIME 5000
#define DEFAULT_SPEED 62500

// Function prototypes
void init_gpio();
void move_forward();
void move_back();
void init_motor_parts(uint *slice_num_1, uint *slice_num_2);
void steer_left(uint *slice_num_1, uint *slice_num_2);
void steer_right(uint *slice_num_1, uint *slice_num_2);

#endif