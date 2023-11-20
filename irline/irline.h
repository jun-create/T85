#ifndef irline_h
#define irline_h
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
// for bar code scanning
#define SAMPLE_COUNT 2048
#define SAMPLE_RATE 100         // in ms
#define BARCODE_CHECK_TIME 2000 // in ms
// tasks
#define irline_TASK_MESSAGE_BUFFER_SIZE (60)

// for every bit of the barcode, send it into the message buffer for conversion
extern MessageBufferHandle_t xControl_barcode_buffer;

// for conversion of binary decrypted to text, send it into the message buffer to send to wifi
extern MessageBufferHandle_t xControl_wifi_print_buffer;

/*
 * Converts binary to decimal
 * for converting binary bits of the barcode to decimal
 */
int binaryToDecimal(char *binary);

/*
 * Get time in milliseconds
 * for measuring the pulse width of the barcode
 */
long long getTimeMS(void);

/*
 * Checks if the adc value, or the ir sensor is higher than NEUTRAL_VALUE(800)
 * Sets ir_digital_value to high(1) or low(0) accordingly
 */
int8_t is_high();

/*
 * Debouncer for the ir sensor, or the adc value
 */
bool debounced(int8_t value);

/*
 * Reads the adc value
 * If the adc value does not change to LOW, it means it is detecting a line
 * returns callback true if callback is terminated
 * * returns callback early if adc value does not match the last state
 * * returns callback later if adc value is properly debounced
 */
bool repeating_sample_adc(struct repeating_timer *t);
/*
 * Read and return raw adc value, ir_digital_value
 */
bool repeating_sample_adc_r(struct repeating_timer *t);

/*
 * Digitalise the bar code signal from an array
 * convert binary of bar code into decimal
 */
void bin_barcode_collection_task(__unused void *params);
void convert_bin_to_text_task(__unused void *params);
void gpio_callback(uint gpio, uint32_t events);
void wifi_gpio_callback(uint gpio, uint32_t events);
/*
 * Initialise the ADC and GPIO pins
 */
void init_adc();
int line();
int barcode();
int barcodeTest();
int barcodeOverWifi();
#endif