// This example uses the ultrasonic script to get centimeters and writes it to UART

#include "pico/stdlib.h"
#include <stdio.h>
#include "ultrasonic.h"
#include "hardware/uart.h"

uint trigPin = 0;
uint echoPin = 1;

int main()
{
    stdio_init_all();
    setupUltrasonicPins(trigPin, echoPin);
    while (true)
    {
        printf("\n%d cm", getCm(trigPin, echoPin));
        sleep_ms(300);
        if (detect_near_object(trigPin, echoPin))
        {
            printf("Near object detected");
            continue;
        }
        printf("No object detected");
    }
}