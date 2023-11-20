
/* IR Sensor: Detect line for line following and measure pulse width for barcode.
 * High value (1) means detected line/barcode, Low value (0) means no line/barcode
 */

/* Feed the GP2 into the ADC at GP26 */
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <string.h>

// include header
#include "irline.h"

// for passing message to wifi
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

// define PWM and ADC pins
#define ADC_PIN_LEFT 28  // not used right now
#define ADC_PIN_RIGHT 27 // not used right now
#define ADC_PIN_CENTER 0

#define PWM_FREQ 20
#define PWM_DUTY_CYCLE 50
#define ADC_SAMPLING_PERIOD 25 // in ms

// digitalise the analog signal
#define NEUTRAL_VALUE 800
#define DEBOUNCE_COUNT 3
volatile static int8_t ir_digital_value = 0;

// for bar code scanning
#define SAMPLE_COUNT 2048
#define SAMPLE_RATE 100         // in ms
#define BARCODE_CHECK_TIME 2000 // in ms
// tasks
#define irline_TASK_MESSAGE_BUFFER_SIZE (60)
// for every bit of the barcode, send it into the message buffer for conversion
MessageBufferHandle_t xControl_barcode_buffer;
// for conversion of binary decrypted to text, send it into the message buffer to send to wifi
MessageBufferHandle_t xControl_wifi_print_buffer;

// calculations
static long long event_arr[SAMPLE_COUNT];
static volatile long long start_time = 0;
static volatile long long end_time = 0;
static volatile long long transverse_width = 1;
static volatile long long old_transverse_width = 1;
// assume speed of 10cm/s
static volatile double speed = 10;

volatile uint16_t sample_counter = 0;
/*
 * Converts binary(string) to decimal
 * for converting binary bits of the barcode to decimal
 * (with online support)
 */
int binaryToDecimal(char *binary)
{
    char *num = binary;
    int dec_value = 0;

    // Initializing base value to 1
    int base = 1;

    int len = strlen(num);
    for (int i = len - 1; i >= 0; i--)
    {
        if (num[i] == '1')
            dec_value += base;
        base = base * 2;
    }

    return dec_value;
}

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

/*
 * Checks if the adc value, or the ir sensor is higher than NEUTRAL_VALUE(800)
 * Sets ir_digital_value to high(1) or low(0) accordingly
 */
int8_t is_high()
{
    if (adc_read() > NEUTRAL_VALUE) // if adc read is higher than neutral value, then it is high
    {
        ir_digital_value = 1;
        return 1;
    }
    // if adc read is lower than neutral value, then it is low
    ir_digital_value = 0;
    return 0;
}

/*
 * Debouncer for the ir sensor, or the adc value
 */
bool debounced(int8_t value)
{
    static int8_t count = 0;
    static int8_t last_state = 0;
    if (value != last_state)
    {
        count = 0;
    }
    else
    {
        if (count < DEBOUNCE_COUNT)
        {
            count++;
        }
        else
        {
            count = 0;
            return true;
        }
    }
    last_state = value;
    return false;
}

/*
 * Reads the adc value
 * If the adc value does not change to LOW, it means it is detecting a line
 * returns callback true if callback is terminated
 * * returns callback early if adc value does not match the last state
 * * returns callback later if adc value is properly debounced
 */
bool repeating_sample_adc(struct repeating_timer *t)
{
    // check if value is debounced, otherwise returns to repeating timer
    if (!debounced(is_high()))
    {
        return true;
    }
    printf("[adc_digi] %d\n", ir_digital_value);

    // print if it is high or low
    if (ir_digital_value)
    {
        printf("[adc_digi] HIGH\n");
    }
    else
    {
        printf("[adc_digi] LOW\n");
    };
    return true;
}

/*
 * Read and return raw adc value, ir_digital_value
 */
bool repeating_sample_adc_r(struct repeating_timer *t)
{
    is_high(adc_read());
    printf("[adc_raw] %d\n", adc_read());
    return true;
}
/* Tries to detect a barcode
 */
static bool state = false;
/*
 * Tries to detect a barcode
 */
void gpio_callback(uint gpio, uint32_t events)
{
    // catch single event to ensure we don't get spurious interrupts
    if (events == GPIO_IRQ_EDGE_RISE)
    {
        // the timer has fired
        if (state == true)
        {
            return;
        }
        // detect black bar, start timer
        state = true;
        printf("[barcode] Black detected!\n");
        start_time = getTimeMS();
        return;
    }

    // catch single event to ensure we don't get spurious interrupts
    if (events == GPIO_IRQ_EDGE_FALL)
    {
        if (state == false)
        {
            return;
        }
        // detect white space, end timer, calculate pulse width
        state = false;
        printf("[barcode] White detected!\n");
        end_time = getTimeMS();
        // record time taken to travel
        long long transverse_width = end_time - start_time;
        double distance_in_cm = ((double)transverse_width / 1000) * speed;
        printf("[barcode] Transverse width: %lld\n", transverse_width);
        printf("[barcode] Distance: %f\n", distance_in_cm);
        if (distance_in_cm > 2)
        {
            printf("Thick line\n");
        }
        else
        {
            printf("Thin line\n");
        }
        // add to array
        event_arr[sample_counter] = transverse_width;
        sample_counter++;
        return;
    }
    return;
}

void wifi_gpio_callback(uint gpio, uint32_t events)
{
    size_t xBytesSent;
    char black_thick[5] = "111";
    char black_thin[5] = "1";
    // char white_thick[5] = "000";
    // char white_thin[5] = "0";
    char *pcStringToSend;

    // catch single event to ensure we don't get spurious interrupts
    if (events == GPIO_IRQ_EDGE_RISE)
    {
        // the timer has fired
        if (state == true)
        {
            return;
        }
        //
        // detect black bar, start timer
        state = true;
        printf("[barcode] Black detected!\n");
        start_time = getTimeMS();
        return;
    }

    // catch single event to ensure we don't get spurious interrupts
    if (events == GPIO_IRQ_EDGE_FALL)
    {
        if (state == false)
        {
            return;
        }
        // detect white space, end timer, calculate pulse width
        state = false;
        printf("[barcode] White detected!\n");
        end_time = getTimeMS();
        // record time taken to travel
        long long transverse_width = end_time - start_time;
        double distance_in_cm = ((double)transverse_width / 1000) * speed;
        printf("[barcode] Transverse width: %lld\n", transverse_width);
        printf("[barcode] Distance: %f\n", distance_in_cm);
        if (distance_in_cm > 2)
        {
            printf("Thick line\n");
            // save to message buffer
            pcStringToSend = (char *)malloc(strlen(black_thick) + 1);
            strcpy(pcStringToSend, black_thick);
        }
        else
        {
            printf("Thin line\n");
            // save to message buffer
            pcStringToSend = (char *)malloc(strlen(black_thin) + 1);
            strcpy(pcStringToSend, black_thin);
        }

        // send from isr and clean up
        xBytesSent = xMessageBufferSend(xControl_barcode_buffer,
                                        (void *)&pcStringToSend,
                                        strlen(pcStringToSend),
                                        pdFALSE);
        free(pcStringToSend);

        // error handling
        if (xBytesSent != strlen(pcStringToSend))
        {
            /* The string could not be added to the message buffer because there was
            not enough free space in the buffer. */
        }

        // add to array
        event_arr[sample_counter] = transverse_width;
        sample_counter++;
        return;
    }
    return;
}
/*
 * Task to collect binary from irsensor
 */
void bin_barcode_collection_task(__unused void *params)
{
    char *fReceivedData;                            // data for each bit of the barcode when interrupt
    char barcode_capture_string[SAMPLE_COUNT] = ""; // need function to reset this barcode_capture_string
    while (true)
    {
        // Receive data
        xMessageBufferReceive(
            xControl_barcode_buffer, /* The message buffer to receive from. */
            (void *)&fReceivedData,  /* Location to store received data. */
            sizeof(fReceivedData),   /* Maximum number of bytes to receive. */
            portMAX_DELAY);          /* Wait indefinitely */
        // Collect data
        strcat(barcode_capture_string, fReceivedData); // add to string
        fReceivedData = "";                            // reset fReceivedData

        // send to conversion_task
        xMessageBufferSend(xControl_barcode_buffer,
                           (void *)&barcode_capture_string,
                           sizeof(barcode_capture_string),
                           0);
    }
}

/*
 * Task to decrypt binary to text via code39
 */
void convert_bin_to_text_task(__unused void *params)
{
    // Variables to receive data from the message buffer
    char *fReceivedData;
    while (true)
    {
        // Receive data
        xMessageBufferReceive(
            xControl_barcode_buffer, /* The message buffer to receive from. */
            (void *)&fReceivedData,  /* Location to store received data. */
            sizeof(fReceivedData),   /* Maximum number of bytes to receive. */
            portMAX_DELAY);          /* Wait indefinitely */

        // Do processing
        printf("[task] %s\n", fReceivedData);

        // Send to Wifi
        printf("[barcode] %s\n", fReceivedData);
        xMessageBufferSend(xControl_wifi_print_buffer, (void *)&fReceivedData, sizeof(fReceivedData), 0);
    }
}
/*
 * Initialise the ADC and GPIO pins
 */
void init_adc()
{
    stdio_init_all();
    adc_init();
    // gpio_init(ADC_PIN_LEFT);
    // gpio_init(ADC_PIN_RIGHT);
    gpio_init(ADC_PIN_CENTER);
}

int line()
{
    init_adc();
    // set up repeating timer to sample the ir sensor
    struct repeating_timer sampler;
    add_repeating_timer_ms(-500, repeating_sample_adc, NULL, &sampler);

    while (1)
    {
        // tight_loop_contents();
        sleep_ms(1000);
    }
}
// set up to read the adc raw values
int barcodeTest()
{
    struct repeating_timer sampler_r;
    add_repeating_timer_ms(-200, repeating_sample_adc_r, NULL, &sampler_r);

    return 0;
    while (1)
    {
        // tight_loop_contents();
        sleep_ms(1000);
    }
}
// in use for local testing, set up interrupt to sample black bars
int barcode()
{
    init_adc();
    gpio_set_irq_enabled_with_callback(ADC_PIN_CENTER, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    while (1)
    {
        // tight_loop_contents();
        sleep_ms(1000);
    }
    return 0;
}

// coding for final product
int barcodeOverWifi()
{
    init_adc();
    gpio_set_irq_enabled_with_callback(ADC_PIN_CENTER, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &wifi_gpio_callback);

    vTaskStartScheduler();
    return 0;
}