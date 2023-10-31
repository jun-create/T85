// Get readings from ultrasonic sensor

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

int timeout = 26100;
// distance for objects to be considered near
#define NEAR_DISTANCE 10

//
volatile absolute_time_t start_time;
volatile absolute_time_t end_time;
volatile bool echo_received;

void echo_callback(uint gpio, uint32_t events);

void setupUltrasonicPins(uint trigPin, uint echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_callback);
}

/*
 * Callback function for echo pin
 * Sets start and end time for pulse
 */
void echo_callback(uint gpio, uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE)
    {
        start_time = get_absolute_time();
    }
    else if (events == GPIO_IRQ_EDGE_FALL)
    {
        end_time = get_absolute_time();
        echo_received = true;
    }
}

uint64_t getPulse(uint trigPin, uint echoPin)
{
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);
    while (!echo_received)
        tight_loop_contents();

    return absolute_time_diff_us(start_time, end_time);
}

float getCm(uint trigPin, uint echoPin)
{
    float pulseLength = (float)getPulse(trigPin, echoPin);
    return pulseLength / 29 / 2;
}

uint64_t getInch(uint trigPin, uint echoPin)
{
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    return (long)pulseLength / 74.f / 2.f;
}

int detect_near_object(uint trigPin, uint echoPin)
{
    if (getCm(trigPin, echoPin) < NEAR_DISTANCE)
    {
        return 1;
    }
    return 0;
}