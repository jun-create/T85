#include "irline.h"
#include "wifi.h"

static const unsigned char bit_reverse_table256[] =
    {
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

long long get_time_ms(void)
{
    struct timeval timevalue;

    gettimeofday(&timevalue, NULL);
    return (((long long)timevalue.tv_sec) * 1000) + (timevalue.tv_sec / 1000);
}

char read_char(char bars, char spaces)
{
    static char CODE39ENCODE[] = "~1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-. *";
    char bar_num = 0, space_num = 1;
    switch (bars)
    {
    case 0b10001:
        bar_num = 1;
        break;
    case 0b01001:
        bar_num = 2;
        break;
    case 0b11000:
        bar_num = 3;
        break;
    case 0b00101:
        bar_num = 4;
        break;
    case 0b10100:
        bar_num = 5;
        break;
    case 0b01100:
        bar_num = 6;
        break;
    case 0b00011:
        bar_num = 7;
        break;
    case 0b10010:
        bar_num = 8;
        break;
    case 0b01010:
        bar_num = 9;
        break;
    case 0b00110:
        bar_num = 10;
        break;
    default:
        break;
    }
    switch (spaces)
    {
    case 0b0100:
        space_num = 0;
        break;
    case 0b0010:
        space_num = 10;
        break;
    case 0b0001:
        space_num = 20;
        break;
    case 0b1000:
        space_num = 30;
        break;
    default:
        break;
    }
    printf("[barcode] bars is %d, spaces is %d\n", bars, spaces);
    printf("[barcode] bar_num is %d, space_num is %d\n", bar_num, space_num);
    if (bar_num == 0 || space_num == 1)
        return '%';
    return CODE39ENCODE[bar_num + space_num];
}
char read_char_reversed(char bars, char spaces)
{
    return read_char(bit_reverse_table256[bars] >> 3, bit_reverse_table256[spaces] >> 4);
}

// Define slow speed as 10cm/s
#define SLOW_SPEED 10

char randomtext[100] = "";
char placeholdertext[100] = "\n";
void barcode_handler(uint32_t events)
{
    static volatile int counter = 0;
    static volatile absolute_time_t prev_time = {};
    static char info[2] = {0};
    static char data[10] = "";
    static uint8_t datacount = 0;
    static bool reversed = false;
    absolute_time_t now_time = get_absolute_time();

    int64_t volatile elapsed_time = absolute_time_diff_us(prev_time, now_time);
    static volatile uint64_t small_bar_threshold = 0;

    // sprintf(ssss, "Elapse |  [barcode] elapsed time: %llu\n", elapsed_time);
    prev_time = now_time;

    // float distance = ((float)elapsed_time * (float)SLOW_SPEED) / 1000.0;
    if (events == GPIO_IRQ_EDGE_RISE)
    {

        // sprintf(pppp, "[barcode] Black detected!\n");
        if (counter == 0)
        {
            return;
        }
        if (elapsed_time > small_bar_threshold)
        {
            info[SPACE] += SPACE_BIT_VALUE[counter / 2];
        }
        ++counter;
    }
    if (events == GPIO_IRQ_EDGE_FALL)
    {
        if (small_bar_threshold == 0)
            small_bar_threshold = elapsed_time * 2;
        if (elapsed_time > small_bar_threshold)
        {
            info[BAR] += BAR_BIT_VALUE[counter / 2];
        }
        if (++counter == 9)
        {
            if (datacount == 0)
            {
                if (read_char(info[BAR], info[SPACE]) == '*')
                {
                    data[datacount++] = '*';
                }
                else if (read_char_reversed(info[BAR], info[SPACE]) == '*')
                {
                    reversed = true;
                    data[datacount++] = '*';
                }
                else
                {
                    sprintf(randomtext, "[barcode] barcode does not start with '*'\n");
                    xMessageBufferSendFromISR(wifiMsgBufferFromISR, &randomtext, sizeof(randomtext), NULL);
                }
            }
            else
            {
                if (reversed)
                    data[datacount] = read_char_reversed(info[BAR], info[SPACE]);
                else
                    data[datacount] = read_char(info[BAR], info[SPACE]);
                if (data[datacount++] == '*')
                {
                    datacount = 0;
                    xMessageBufferSendFromISR(wifiMsgBufferFromISR, &data, sizeof(data), NULL);
                    memset(&data, 0, sizeof(data));
                    reversed = false;
                }
            }
            xMessageBufferSendFromISR(wifiMsgBufferFromISR, &data, sizeof(data), NULL);
            xMessageBufferSendFromISR(wifiMsgBufferFromISR, &placeholdertext, sizeof(1), NULL);
            counter = 0;
            info[BAR] = 0;
            info[SPACE] = 0;
        }
    }
}

// check if the interrupt from the barcode detection is a barcode
bool is_barcode(absolute_time_t start, absolute_time_t end)
{
    uint64_t time_elapse = absolute_time_diff_us(start, end) / 1000;
    if (time_elapse < 400)
    {
        return true;
    }
    return false;
}

static volatile bool left = false;
static volatile bool right = false;

// handler for wall detection
void wall_detect_handler(uint16_t gpio, uint32_t events)
{
    if (gpio == 26)
    {
        static volatile absolute_time_t leftHit = {};
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            left = true;
            leftHit = get_absolute_time();
        }
        else
        {
            if (!right)
            {
                left_tilt();
                return;
            }
            left = false;
            absolute_time_t left_now_time = get_absolute_time();
            if (!is_barcode(leftHit, left_now_time))
            {
                stop();
                return;
            }
            stop();
        }
    }
    else
    {
        static volatile absolute_time_t rightHit = {};
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            right = true;
            rightHit = get_absolute_time();
        }
        else
        {
            if (!left)
            {
                right_tilt();
                return;
            }
            right = false;
            absolute_time_t right_now_time = get_absolute_time();
            if (!is_barcode(rightHit, right_now_time))
            {
                stop();
                return;
            }
            stop();
        }
    }
}

// taken from https://stackoverflow.com/questions/32410186/convert-bool-array-to-int32-unsigned-int-and-double
char convert_bit_array_to_uint8(bool arr[], int count)
{
    char ret = 0;
    char tmp;
    for (int i = 0; i < count; i++)
    {
        tmp = arr[i];
        ret |= tmp << (count - i - 1);
    }
    return ret;
}

/*
 * Initialise the ADC and GPIO pins
 */
void init_adc()
{
    adc_init();
    gpio_init(ADC_PIN);
    // barcodeMsgBuffer = xMessageBufferCreate(5);
}