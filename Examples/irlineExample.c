// This example uses the ultrasonic script to get centimeters and writes it to UART

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/uart.h"
#include "irline.h"

// int main()
// {
//     barcode();
//     // line();

//     return 0;
// }

int main()
{
    init_adc();
    TaskHandle_t printtask;
    xTaskCreate(convert_bin_to_text_task, "TestTempThread", configMINIMAL_STACK_SIZE, NULL, 8, &printtask);
    xControl_barcode_buffer = xMessageBufferCreate(irline_TASK_MESSAGE_BUFFER_SIZE);

    // set up to read the adc raw values
    // struct repeating_timer sampler_r;
    // add_repeating_timer_ms(-200, repeating_sample_adc_r, NULL, &sampler_r);

    // set up interrupt to sample black bars
    gpio_set_irq_enabled_with_callback(ADC_PIN_CENTER, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    vTaskStartScheduler();

    return 0;
}