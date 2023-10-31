/*
 * This code is cutdown version of the Pico LWIP examples.
 * [MOTOR] Its functions are to control the motor of the embedded systems car.
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <sys/time.h>

// Distance between two holes on the encoder 21cm / 20 holes
static const float WHEEL_HOLE_DIST = 21 / 20;

// encoder
long long a_falling_edge = 0;
long long b_falling_edge = 0;
long long speed_ms;
long long time_diff_ms = 0;
float distance = 0;
float speed;
int count = 0;

/*
 * Get time in milliseconds
 * for measuring the pulse width of the barcode
 */
long long getTimeMS(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

// callback function for encoder
void gpio_callback(uint gpio, uint32_t events)
{
    a_falling_edge = b_falling_edge;
    b_falling_edge = getTimeMS();
    speed_ms = b_falling_edge - a_falling_edge;
    time_diff_ms = speed_ms;

    distance += count * WHEEL_HOLE_DIST;
    speed = (WHEEL_HOLE_DIST / (double)time_diff_ms) * 1000;
    printf("Distance: %fcm\n", distance);
    printf("Speed: %fcm/s\n", speed);
}
