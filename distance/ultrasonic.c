//Get readings from ultrasonic sensor

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/timer.h"

int timeout = 26100;

volatile absolute_time_t startTime;
volatile absolute_time_t endTime;
volatile uint64_t pulseLength;
volatile bool echo_received;

void echocallback(uint32_t events)
{
    if (events == GPIO_IRQ_EDGE_RISE)
    {
        startTime = get_absolute_time();
    }
    else if (events == GPIO_IRQ_EDGE_FALL)
    {
        endTime = get_absolute_time();

        pulseLength = absolute_time_diff_us(startTime, endTime);
        
    }
}

void sendpulse(uint trigPin, uint echoPin)
{
    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

}

void setup_ultrasonic_pins(uint trigPin, uint echoPin)
{
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
    // gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echoCallback);

}

double getcm(uint trigPin, uint echoPin)
{
    sendpulse(trigPin, echoPin);
    return pulseLength / 29 / 2;
}
