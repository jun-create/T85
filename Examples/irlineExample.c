// This example uses the ultrasonic script to get centimeters and writes it to UART

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/uart.h"
#include "irline.h"

int main()
{
    barcode();
    // line();

    return 0;
}