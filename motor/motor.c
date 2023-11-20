/*
 * This code is cutdown version of the Pico LWIP examples.
 * [MOTOR] Its functions are to control the motor of the embedded systems car.
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <sys/time.h>
#include "encoder.h"

// Left Motor
#define ENA_PIN 10
// Left Wheel Control 1 (CW)
#define IN1_PIN 11
// Left Wheel Control 2 (CCW)
#define IN2_PIN 12

// Right Motor
#define ENB_PIN 13
// Right Wheel Control 3 (CW)
#define IN3_PIN 14
// Right Wheel Control 4 (CCW)
#define IN4_PIN 15

// Wheel size
#define CIRCUMFERENCE 21

// Turn time in ms
#define MOVE_TIME 5000
#define DEFAULT_SPEED 62500

void init_gpio()
{
    gpio_init(0);
    gpio_init(1);

    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_init(IN3_PIN);
    gpio_init(IN4_PIN);

    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_set_dir(IN3_PIN, GPIO_OUT);
    gpio_set_dir(IN4_PIN, GPIO_OUT);

    gpio_set_dir(0, GPIO_OUT);
    gpio_set_dir(1, GPIO_OUT);
    gpio_put(0, 1);
    gpio_put(1, 1);
}

void move_forward()
{
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);
    gpio_put(IN3_PIN, 1);
    gpio_put(IN4_PIN, 0);
}

void move_back()
{
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 1);
    gpio_put(IN3_PIN, 0);
    gpio_put(IN4_PIN, 1);
}

// initialize the motor
void init_motor_parts(uint *slice_num_1, uint *slice_num_2)
{
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    gpio_set_function(ENB_PIN, GPIO_FUNC_PWM);

    pwm_set_clkdiv(*slice_num_1, 100);
    pwm_set_clkdiv(*slice_num_2, 100);

    pwm_set_wrap(*slice_num_1, DEFAULT_SPEED);
    pwm_set_wrap(*slice_num_2, DEFAULT_SPEED);

    pwm_set_chan_level(*slice_num_1, PWM_CHAN_A, DEFAULT_SPEED / 2);
    pwm_set_chan_level(*slice_num_2, PWM_CHAN_B, DEFAULT_SPEED / 2);

    pwm_set_enabled(*slice_num_1, true);
    pwm_set_enabled(*slice_num_2, true);
}

/*
 * Turns the left motor less than the right motor
 * to turn the car left
 */
void steer_left(uint *slice_num_1, uint *slice_num_2)
{
    // Slow down the left motor
    pwm_set_chan_level(*slice_num_1, PWM_CHAN_A, DEFAULT_SPEED * 0.2);
    sleep_ms(MOVE_TIME);

    // Reset left motor speed
    pwm_set_chan_level(*slice_num_1, PWM_CHAN_A, DEFAULT_SPEED);
}

/*
 * Turns the right motor less than the left motor
 * to turn the car right
 */
void steer_right(uint *slice_num_1, uint *slice_num_2)
{
    // Slow down the left motor
    pwm_set_chan_level(*slice_num_2, PWM_CHAN_B, DEFAULT_SPEED * 0.2);
    sleep_ms(MOVE_TIME);

    // Reset left motor speed
    pwm_set_chan_level(*slice_num_2, PWM_CHAN_B, DEFAULT_SPEED);
}

int motor()
{
    init_gpio();
    stdio_init_all();
    gpio_set_irq_enabled_with_callback(9, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // get the slice num
    uint slice_num_1 = pwm_gpio_to_slice_num(ENA_PIN);
    uint slice_num_2 = pwm_gpio_to_slice_num(ENB_PIN);
    init_motor_parts(&slice_num_1, &slice_num_2);

    // while loop to control the car
    while (1)
    {
        move_forward();
        sleep_ms(MOVE_TIME);
        move_back();
        sleep_ms(MOVE_TIME);
        // forward left
        move_forward();
        steer_left(&slice_num_1, &slice_num_2);
        // forward right
        move_forward();
        steer_right(&slice_num_1, &slice_num_2);
    }
    return 0;
}
