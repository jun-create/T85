
/* IR Sensor: Detect line for line following and measure pulse width for barcode.
 * High value (1) means detected line/barcode, Low value (0) means no line/barcode
 */

/* Feed the GP2 into the ADC at GP26 */
#include <sys/time.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/adc.h>

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
static long long event_arr[SAMPLE_COUNT];
static volatile long long start_time = 0;
static volatile long long end_time = 0;
static volatile long long transverse_width = 1;
static volatile long long old_transverse_width = 1;
// assume speed of 10cm/s
static volatile double speed = 10;

volatile uint16_t sample_counter = 0;

/*
 * Converts binary to decimal
 * for converting binary bits of the barcode to decimal
 */
int binaryToDecimal(int decimal)
{
    int binary = 0;
    int i = 0;

    while (decimal != 0)
    {
        binary |= (decimal & 1) << i;
        decimal >>= 1;
        i++;
    }
    return binary;
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

/*
 * Digitalise the bar code signal from an array
 * convert binary of bar code into decimal
 */
void digitalise_barcode(uint8_t *buf)
{
}

/* Tries to detect a barcode
 */
static bool state = false;

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

int barcode()
{
    init_adc();

    // set up to read the adc raw values
    // struct repeating_timer sampler_r;
    // add_repeating_timer_ms(-200, repeating_sample_adc_r, NULL, &sampler_r);

    // set up interrupt to sample black bars
    gpio_set_irq_enabled_with_callback(ADC_PIN_CENTER, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while (1)
    {
        // tight_loop_contents();
        sleep_ms(1000);
    }
    return 0;
}