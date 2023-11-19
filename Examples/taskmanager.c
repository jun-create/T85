#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h" // for temp task
#include <stdio.h>
#include <math.h>

// drivers
// #include "encoder.h"
#include "irline.h"
// #include "magnometer.h"
// #include "motor.h"
// #include "ultrasonic.h"
// #include "wifi.h"

// tasks
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#define NUMBER_OF_DATA_POINTS 10

// for avg task 2
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
MessageBufferHandle_t xControlAverageBuffer;

// for simple avg task 3
MessageBufferHandle_t xControlSimpleAverageBuffer;

// for print task 4
MessageBufferHandle_t xControlAveragePrintBuffer;
MessageBufferHandle_t xControlSimpleAveragePrintBuffer;

// irline task
// MessageBufferHandle_t xControl_irline_buffer;

float read_onboard_temperature()
{

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return tempC;
}

// The first task reads the temperature data from the RP2040's built-in temperature sensor
// and sends it to two tasks every 1 second
void temp_task(__unused void *params)
{
    float temperature = 0.0;
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while (true)
    {
        vTaskDelay(1000);
        temperature = read_onboard_temperature();

        xMessageBufferSend(xControlAverageBuffer, (void *)&temperature, sizeof(temperature), 0);
        xMessageBufferSend(xControlSimpleAverageBuffer, (void *)&temperature, sizeof(temperature), 0);
    }
}

// The second task will perform a moving average on a buffer of ten data points.
// The function will then send it to the print_task to print the average.
void avg_task(__unused void *params)
{
    // Variables to receive data from the message buffer
    float fReceivedData;

    // Variables to do processing with
    static float data[NUMBER_OF_DATA_POINTS] = {0};
    static int index = 0;
    static int count = 0;
    float sum = 0;

    // Variables to send data to the message buffer
    float avg_temperature = 0.0;

    while (true)
    {
        // Receive data
        xMessageBufferReceive(
            xControlAverageBuffer,  /* The message buffer to receive from. */
            (void *)&fReceivedData, /* Location to store received data. */
            sizeof(fReceivedData),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);         /* Wait indefinitely */

        // Do processing (this is from the exercise)
        sum -= data[index];                          // Subtract the oldest element from sum
        data[index] = fReceivedData;                 // Assign the new element to the data
        sum += data[index];                          // Add the new element to sum
        index = (index + 1) % NUMBER_OF_DATA_POINTS; // Update the index- make it circular

        if (count < NUMBER_OF_DATA_POINTS)
            count++; // Increment count till it reaches NUMBER_OF_DATA_POINTS count
        avg_temperature = sum / count;

        // Send data to another message buffer
        xMessageBufferSend(xControlAveragePrintBuffer, (void *)&avg_temperature, sizeof(avg_temperature), 0);
    }
}

// The third task will perform a simple averaging.
// The function will then send it to the print_task to print the simple average.
void simple_avg_task(__unused void *params)
{
    // Variables to receive data from the message buffer
    float fReceivedData;

    // Variables to do processing with
    static int count = 0;
    float sum = 0;

    // Variables to send data to the message buffer
    float simple_avg_temperature = 0.0;

    while (true)
    {
        // Receive data
        xMessageBufferReceive(
            xControlSimpleAverageBuffer, /* The message buffer to receive from. */
            (void *)&fReceivedData,      /* Location to store received data. */
            sizeof(fReceivedData),       /* Maximum number of bytes to receive. */
            portMAX_DELAY);              /* Wait indefinitely */

        // Do processing
        sum += fReceivedData;
        count++;
        simple_avg_temperature = sum / count;

        // Send data to another message buffer
        xMessageBufferSend(xControlSimpleAveragePrintBuffer, (void *)&simple_avg_temperature, sizeof(simple_avg_temperature), 0);
    }
}

// The fourth task is exclusively for executing all the printf statements.
// This function receives the data from the avg_task, simple_avg_task and print it.
void print_task(void *params)
{
    // Variables to receive data from the message buffer
    float fReceivedData_avg_temp;
    float fReceivedData_simple_avg_temp;
    char *fReceivedData_line_size;

    while (true)
    {
        // Receive data
        xMessageBufferReceive(
            xControlAveragePrintBuffer,      /* The message buffer to receive from. */
            (void *)&fReceivedData_avg_temp, /* Location to store received data. */
            sizeof(fReceivedData_avg_temp),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);                  /* Wait indefinitely */
        xMessageBufferReceive(
            xControlSimpleAveragePrintBuffer,       /* The message buffer to receive from. */
            (void *)&fReceivedData_simple_avg_temp, /* Location to store received data. */
            sizeof(fReceivedData_simple_avg_temp),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);                         /* Wait indefinitely */

        xMessageBufferReceive(
            xControl_irline_buffer,           /* The message buffer to receive from. */
            (void *)&fReceivedData_line_size, /* Location to store received data. */
            sizeof(fReceivedData_line_size),  /* Maximum number of bytes to receive. */
            portMAX_DELAY);                   /* Wait indefinitely */

        // Print data
        printf("[Temp] Average: %0.2f C\n", fReceivedData_avg_temp);
        printf("[Temp] Simple Average: %0.2f C\n", fReceivedData_simple_avg_temp);
        printf("[irline] line size: %s\n", fReceivedData_line_size);
    }
}

void vLaunch(void)
{
    TaskHandle_t temptask;
    xTaskCreate(temp_task, "TestTempThread", configMINIMAL_STACK_SIZE, NULL, 8, &temptask);
    TaskHandle_t avgtask;
    xTaskCreate(avg_task, "TestAvgThread", configMINIMAL_STACK_SIZE, NULL, 9, &avgtask);
    TaskHandle_t simpleavgtask;
    xTaskCreate(simple_avg_task, "TestSimpleAvgThread", configMINIMAL_STACK_SIZE, NULL, 10, &simpleavgtask);
    TaskHandle_t printtask;
    xTaskCreate(print_task, "TestPrintThread", configMINIMAL_STACK_SIZE, NULL, 11, &printtask);

    // task operations examples
    // vTaskSuspend(irline_task_handle);
    // vTaskResume(irline_task_handle);
    // vTaskDelete(irline_task_handle);

    xControlAverageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    xControlSimpleAverageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    xControlAveragePrintBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    xControlSimpleAveragePrintBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    // irline
    TaskHandle_t irline_task_handle;
    xTaskCreate(irline_task, "TestIrlineThread", configMINIMAL_STACK_SIZE, NULL, 11, &irline_task_handle);
    xControl_irline_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    vLaunch();
    return 0;
}
