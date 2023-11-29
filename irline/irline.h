#ifndef IRLLINE_H
#define IRLLINE_H

#include <sys/time.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include "motor.h"
#include "FreeRTOS.h"  // Include the FreeRTOS library for real-time operating system functionality.
#include "message_buffer.h"

#define ADC_PIN 15

#define WIDE 1
#define NARROW 0

#define BAR 0
#define SPACE 1
#define BARCODE_BARCOUNT 5
#define BARCODE_SPACECOUNT 4

const static char SPACE_BIT_VALUE[] = {8, 4, 2, 1};
const static char BAR_BIT_VALUE[] = {16, 8, 4, 2, 1};

extern MessageBufferHandle_t barcodeMsgBuffer;
void barcode_handler(uint32_t events);
void init_adc();
void wall_detect_handler(uint16_t gpio, uint32_t events);

#endif