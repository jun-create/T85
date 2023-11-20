#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h" // for temp task
#include <stdio.h>
#include <math.h>

// drivers header files
// #include "encoder.h"
#include "irline.h"
// #include "magnometer.h"
// #include "motor.h"
// #include "ultrasonic.h"
#include "wifi.h"

// tasks
#include "FreeRTOSConfig.h"
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
void barcode_task(__unused void *params)
{
    barcodeOverWifi();
}
// The fourth task is exclusively for executing all the printf statements.
// This function receives the data from the avg_task, simple_avg_task and print it.
void print_task(void *params)
{
    // Variables to receive data from the message buffer
    float fReceivedData_avg_temp;
    float fReceivedData_simple_avg_temp;

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

        // Print data
        printf("[Temp] Average: %0.2f C\n", fReceivedData_avg_temp);
        printf("[Temp] Simple Average: %0.2f C\n", fReceivedData_simple_avg_temp);
    }
}

void wifi_task(__unused void *params)
{
    stdio_init_all();
}

void vLaunch(void)
{
    // wifi task
    TaskHandle_t wifi_task_hdle;
    xTaskCreate(wifi, "TestWifiThread", configMINIMAL_STACK_SIZE, NULL, 8, &wifi_task_hdle);

    // TaskHandle_t temptask;
    // xTaskCreate(temp_task, "TestTempThread", configMINIMAL_STACK_SIZE, NULL, 8, &temptask);
    // TaskHandle_t avgtask;
    // xTaskCreate(avg_task, "TestAvgThread", configMINIMAL_STACK_SIZE, NULL, 9, &avgtask);
    // TaskHandle_t simpleavgtask;
    // xTaskCreate(simple_avg_task, "TestSimpleAvgThread", configMINIMAL_STACK_SIZE, NULL, 10, &simpleavgtask);
    // TaskHandle_t printtask;
    // xTaskCreate(print_task, "TestPrintThread", configMINIMAL_STACK_SIZE, NULL, 11, &printtask);

    // task operations examples
    // vTaskSuspend(irline_task_handle);
    // vTaskResume(irline_task_handle);
    // vTaskDelete(irline_task_handle);

    // xControlAverageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    // xControlSimpleAverageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    // xControlAveragePrintBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    // xControlSimpleAveragePrintBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    /*
     * Straight Path    (Motor, Encoder, IR) [Max 4 marks]
     * - [4] Perfect straight drive
     * - [3] Snaking but finishing without touching a line
     * - [2] Wheel touch the line but finish
     * - [1] Reasonable attempt
     * 1. get line size from irline
     * 2. if right side ir sensor touches the line,
     * correct the motor and encoder to steer a bit left for 2-3 units of encoder
     * 3. if left side ir sensor touches the line,
     * correct the motor and encoder to steer a bit right for 2-3 ticks of encoder
     * 4. have motor run at constant speed
     */

    /*
     * Right/left-angle turn (Motor, Encoder, IR, Magnetometer) [Max 4 marks]
     * - [4] Demonstrate move to a corner, turn and continue moving straight without touching the line for some distance
     * - [3] Demonstrate move to a corner and turn only without touching the line
     * - [2] Demonstrate move to a corner and turns successfully but touching the line
     * - [1] Reasonable attempt
     */

    /*
     * Drive through and detect barcode-decode and send to WiFi (Motor, Encoder, IR, WiFi) [Max 4 marks]
     * - [4] Motor-driven and detects barcode correctly in both directions and displays via WiFi (for all three sizes)
     * - [3] Motor-driven and detects barcode correctly in a single direction and displays via WiFi (for all three sizes)
     * - [2] Hand-driven detects and decodes barcode successfully (for the largest size)
     * - [1] Reasonable attempt
     *
     * 1. Motor runs at constant speed
     * 2. Irline detects the line size
     * 3. Records and stores the line sizes
     * 4. When irline sensor detects the end of the line,
     * 5. saves the barcode value as binary,
     * 6. converts binary to via code39
     * 7. sends converted code to the WiFi
     */
    // task to run
    // irline
    // TaskHandle_t irline_task_handle;
    // xTaskCreate(irline_task, "TestIrlineThread", configMINIMAL_STACK_SIZE, NULL, 11, &irline_task_handle);
    // xControl_irline_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    // motor speed task

    // init
    xControl_barcode_buffer = xMessageBufferCreate(irline_TASK_MESSAGE_BUFFER_SIZE);
    // inits interrupt for scanning
    TaskHandle_t barcode_task_hdle;
    xTaskCreate(barcode_task, "BarcodeCollectionThread", configMINIMAL_STACK_SIZE, NULL, 8, &barcode_task_hdle);

    // collate whole barcode via xControl_barcode_buffer handle
    TaskHandle_t barcode_collection_task_hdle;
    xTaskCreate(bin_barcode_collection_task, "BarcodeCollectionThread", configMINIMAL_STACK_SIZE, NULL, 8, &barcode_collection_task_hdle);

    // convert from binary to text task
    TaskHandle_t conversion_task_hdle;
    xTaskCreate(convert_bin_to_text_task,
                "ConvertBinToTextThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                8,
                &conversion_task_hdle);
    // send to wifi, to print task
    TaskHandle_t wifi_to_print_task;
    xTaskCreate(convert_bin_to_text_task,
                "SendViaThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                8,
                &wifi_to_print_task);

    /*
     * Drive through and detect obstacles and Stop (motor, encoder, IR, Ultrasonic) [Max 3 marks]
     * - [3] Motor-driven and detect obstacles and reverse/U-turn
     * - [2] Motor-driven and detect obstacles and stop
     * - [1] Reasonable attempt *
     */

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();
    // vLaunch();
    wifi();
    return 0;
}
