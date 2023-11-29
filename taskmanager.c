// commands
// p for pause
// f for forward
// t for turn
// s for transition to reverse
// r for reverse
/*
 * taskmanager.c - documentations and commands for tcp use
 * enter the following commands in the terminal to control the car:
 * p for stop the car
 * f for move forward
 * t for turn
 * s for transition to reverse
 * r for reverse
 *
 * More tcp commands:
 * fwd100 - move forward for a certain distance
 *
 * More notes: printed lc and lr should be 0 when the car is stationary, otherwise do a manual reset
 */
#include "FreeRTOS.h"
#include "pico/stdlib.h"
#include "pico/mutex.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <sys/time.h>
#include <hardware/adc.h>

#include "irline.h"
#include "motor.h"
#include "ultrasonic.h"
#include "magnometer.h"
#include "wifi.h"

// Ir Sensor Pins
#define IR_LEFT_PIN 26
#define IR_RIGHT_PIN 27
#define DEFAULT_SPEED 62500 * 0.1
#define ECHO_PIN 12
#define TRI_PIN 13
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)
// Buffer handle for type of movement, forward, backward, clockwise, counter clockwise, reverse
MessageBufferHandle_t h_move_mode_buffer;
// Buffer handle for distance
MessageBufferHandle_t h_dist_buffer;
// Buffer handle for turning angle
MessageBufferHandle_t h_turn_buffer;

static volatile float tkp = 0.1, tki = 0, tkd = 0.05;
static volatile float fkp = 0.15, fki = 0, fkd = 0.075;

int volatile current_bearing = 0;
double ultrasonic_reading = 9999999;
bool b_left_IR_black = false;
bool b_right_IR_black = false;
auto_init_mutex(wifiMutex);

// check if there is an interrupt
inline bool is_interrupt()
{
    int num = 0;
    asm(
        "mrs %[num], ipsr\n"
        : [num] "=rm"(num));
    return num == 0;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{ // Receive data from the TCP connection.
    if (!p)
    {                                      // Check if the received packet buffer is invalid.
        printf("Something went wrong!\n"); // Print an error message.
        return ERR_BUF;
    }
    if (strlen(p->payload) == 0)
    {                 // Check if the received data is empty.
        pbuf_free(p); // Free the packet buffer.
        return ERR_OK;
    }
    else if (p->tot_len > 0)
    {                                                                    // Check if the total length of received data is greater than 0.
        printf("Buffer value: %s, len is %d\n", p->payload, p->tot_len); // Print the received data.
        if (strncmp(p->payload, "start", 5) == 0)
        {
            printf("starting\n");
        }
        if (strncmp(p->payload, "turncw", 6) == 0)
        {
            printf("turn cw\n");
            int new_bearing = 90;
            xMessageBufferSend(h_move_mode_buffer, "t", sizeof(char), 0);
            xMessageBufferSend(h_turn_buffer, &new_bearing, sizeof(new_bearing), 0);
        }
        if (strncmp(p->payload, "turnccw", 7) == 0)
        {
            printf("turn ccw\n");
            int new_bearing = -90;
            xMessageBufferSend(h_move_mode_buffer, "t", sizeof(char), 0);
            xMessageBufferSend(h_turn_buffer, &new_bearing, sizeof(new_bearing), 0);
        }
        if (strncmp(p->payload, "stop", 4) == 0)
        {
            int new_bearing = 0;
            xMessageBufferSend(h_move_mode_buffer, "t", sizeof(char), 0);
            xMessageBufferSend(h_turn_buffer, &new_bearing, sizeof(new_bearing), 0);
        }
        if (strncmp(p->payload, "set", 3) == 0)
        {
            char value[5] = "";
            strncpy(value, p->payload + 4, 4); // 2d.p.
            printf("value is %s\n", value);
            printf("char is %c\n", *(char *)(p->payload + 3));
            switch (*(char *)(p->payload + 3))
            {
            case 'p':
                tkp = atof(value) / 10;
                break;
            case 'i':
                tki = atof(value) / 10;
                break;
            case 'd':
                tkd = atof(value) / 10;
                break;
            case '1':
                fkp = atof(value) / 10;
                break;
            case '2':
                fki = atof(value) / 10;
                break;
            case '3':
                fkd = atof(value) / 10;
                break;
            }
        }
        if (strncmp(p->payload, "fwd", 3) == 0)
        {
            char value[4] = "";
            strncpy(value, p->payload + 3, 4);
            int dist = atoi(value);
            xMessageBufferSend(h_move_mode_buffer, "f", sizeof(char), 0);
            xMessageBufferSend(h_dist_buffer, &dist, sizeof(dist), 0);
        }
        if (strncmp(p->payload, "bar", 3) == 0)
        {
            int dist = 200;
            xMessageBufferSend(h_move_mode_buffer, "b", sizeof(char), 0);
            xMessageBufferSend(h_dist_buffer, &dist, sizeof(dist), 0);
        }
        if (strncmp(p->payload, "reset", 5) == 0)
        {
            reset_wheel_encoder();
        }
        while (!mutex_try_enter(&wifiMutex, 0))
        {
            printf("waiting for mutex\n");
            sleep_ms(5);
        }
        xMessageBufferSend(wifiMsgBuffer, "ack\n", 5, 0);
        mutex_exit(&wifiMutex);
    }
    pbuf_free(p); // Free the packet buffer.
    return ERR_OK;
}

// task for receiving forward data from the server
void server_forward_task()
{
    while (1)
    {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 256, PBUF_RAM);
        xMessageBufferReceive(wifiMsgBuffer, (void *)p->payload, 256, portMAX_DELAY);
        tcp_server_send_data(p, myServer);
        pbuf_free(p);
        taskYIELD();
    }
}

// task for interrupt receiving forward data from the server
void server_forward_task_from_ISR()
{
    while (1)
    {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 256, PBUF_RAM);
        while (xMessageBufferReceiveFromISR(wifiMsgBufferFromISR, (void *)p->payload, 256, NULL) == 0)
            taskYIELD();
        tcp_server_send_data(p, myServer);
        pbuf_free(p);
        taskYIELD();
    }
}

// first interrupt handler
void mainIRQhandler(uint gpio, uint32_t events)
{
    if (gpio == left_wheel_encoder_pin)
    {
        left_wheel_encoder_handler(events);
        return;
    }
    if (gpio == right_wheel_encoder_pin)
    {
        right_wheel_encoder_handler(events);
        return;
    }
    if (gpio == ADC_PIN)
    {
        barcode_handler(events);
        return;
    }
    if (gpio == ECHO_PIN)
    {
        echocallback(events);
        return;
    }
}

// magnometer error for turning
float getBearingError(float current, float target)
{
    float error = target - current;
    if (error > 180)
        return error - 360;
    if (error < -180)
        return error + 360;
    return error;
}

// task for sensing
void sense_task(__unused void *param)
{
    while (true)
    {
        current_bearing = heading();
        ultrasonic_reading = getcm(TRI_PIN, ECHO_PIN);
        b_left_IR_black = gpio_get(IR_LEFT_PIN);
        b_right_IR_black = gpio_get(IR_RIGHT_PIN);

        vTaskDelay(10);
    }
}

// task for moving
void move_task(__unused void *params)
{
    int volatile target_bearing = current_bearing;
    int volatile read_bearing = 0;
    int update = 100;
    char steadycount = 50;
    // reading will be that of previous one

    int volatile read_dist = 0;
    uint64_t volatile target_code = 0;
    char mode = 'p';
    // f for forward
    // b for barcode
    // t for turn
    // s for transition to reverse
    // r for reverse
    // p for paused

    int volatile dist_error = 0;
    int volatile dist_last_error = 0;

    int volatile bearing_error = 0;
    int volatile bearing_last_error = 0;
    float volatile intergral = 0;
    float volatile derivative = 0;
    float volatile control = 0;
    printf("task running\n");

    while (1)
    {
        xMessageBufferReceive(h_move_mode_buffer, (void *)&mode, sizeof(mode), 0);
        bearing_error = getBearingError(current_bearing, target_bearing);

        if (mode == 'p')
        {
            stop();
            if (--update == 0)
            {
                update = 100;
                char update_data[100] = "";
                snprintf(update_data, 100, "[P]lc:%llu\tlr:%llu\ttc:%llu\tec:%d\tcb:%d\ttb:%d\teb:%d\n", g_left_wheel_code, g_right_wheel_code, target_code, dist_error, current_bearing, target_bearing, bearing_error);
                while (!mutex_try_enter(&wifiMutex, 0))
                {
                    vTaskDelay(5);
                    stop();
                }
                xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
                mutex_exit(&wifiMutex);
            }
        }

        if (mode == 'f')
        {
            if (xMessageBufferReceive(h_dist_buffer, (void *)&read_dist, sizeof(read_dist), 0))
            {
                if (read_dist > 0)
                {
                    printf("distanceBuffer: %d\n", read_dist);
                    reset_wheel_encoder();
                    target_code = read_dist;
                }
                else if (read_dist == -1)
                {
                    target_code = 0;
                }
            }
            // check for obsticles
            if (ultrasonic_reading < 15)
            {
                target_code = 0;
            }

            dist_last_error = dist_error;
            dist_error = target_code - g_left_wheel_code;
            derivative = dist_error - dist_last_error;
            // Code will increase going backwards too
            if (dist_error < 2)
            {
                control = 0;
                if (--steadycount == 0)
                {
                    if (dist_error < -2)
                    {
                        stop();
                        printf("lc was %llu, rc was %llu, set tc to %d\n", g_left_wheel_code, g_right_wheel_code, target_code);
                        mode = 's'; // transition state to reverse
                        steadycount = 50;
                    }
                    else
                    {
                        mode = 'p';
                        steadycount = 50;
                    }
                }
            }
            else
            {
                control = fkp * dist_error;
                steadycount = 50;
            }
            control += fkd * derivative;
            if (control > 1)
                control = 1;
            set_speed(control * DEFAULT_SPEED);
            if (g_left_wheel_code < g_right_wheel_code)
            {
                right_tilt();
            }
            if (g_left_wheel_code > g_right_wheel_code)
            {
                for (int i = g_left_wheel_code - g_right_wheel_code; i > 0; i -= 1)
                {
                    left_tilt();
                }
            }
            if (b_left_IR_black)
            {
                right_tilt();
                right_tilt();
            }
            if (b_right_IR_black)
            {
                left_tilt();
                left_tilt();
            }
            forward();

            if (--update == 0)
            {
                update = 100;
                char update_data[100] = "";
                uint16_t speed = control * DEFAULT_SPEED;
                snprintf(update_data, 100, "[FWD]lc:%llu\tlr:%llu\ttar:%llu\terr:%d\tctrl:%.2f\tp:%.3f\td:%.2f\tspeed:%d\n", g_left_wheel_code, g_right_wheel_code, target_code, dist_error, control, fkp, derivative, speed);
                while (!mutex_try_enter(&wifiMutex, 0))
                {
                    vTaskDelay(5);
                    stop();
                }
                xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
                mutex_exit(&wifiMutex);
            }
        }

        if (mode == 'b')
        {
            if (xMessageBufferReceive(h_dist_buffer, (void *)&read_dist, sizeof(read_dist), 0))
            {
                if (read_dist > 0)
                {
                    printf("distanceBuffer: %d\n", read_dist);
                    reset_wheel_encoder();
                    target_code = read_dist;
                }
            }
            // check for obsticles
            if (ultrasonic_reading < 15)
            {
                target_code = 0;
            }

            dist_last_error = dist_error;
            dist_error = target_code - g_left_wheel_code;
            derivative = dist_error - dist_last_error;
            // Code will increase going backwards too
            if (dist_error < 2)
            {
                control = 0;
                if (--steadycount == 0)
                {
                    if (dist_error < -2)
                    {
                        stop();
                        printf("lc was %llu, rc was %llu, set tc to %d\n", g_left_wheel_code, g_right_wheel_code, target_code);
                        mode = 's'; // transition state to reverse
                        steadycount = 50;
                    }
                    else
                    {
                        mode = 'p';
                        steadycount = 50;
                    }
                }
            }
            else
            {
                control = fkp * dist_error;
                steadycount = 50;
            }
            control += fkd * derivative;
            if (control > 1)
                control = 1;
            set_speed(control * DEFAULT_SPEED);
            if (g_left_wheel_code < g_right_wheel_code)
            {
                right_tilt();
            }
            if (g_left_wheel_code > g_right_wheel_code)
            {
                for (int i = g_left_wheel_code - g_right_wheel_code; i > 0; i -= 1)
                {
                    left_tilt();
                }
            }
            forward();

            if (--update == 0)
            {
                update = 100;
                char update_data[100] = "";
                uint16_t speed = control * DEFAULT_SPEED;
                snprintf(update_data, 100, "[BAR]lc:%llu\tlr:%llu\ttar:%llu\terr:%d\tctrl:%.2f\tp:%.3f\td:%.2f\tspeed:%d\n", g_left_wheel_code, g_right_wheel_code, target_code, dist_error, control, fkp, derivative, speed);
                while (!mutex_try_enter(&wifiMutex, 0))
                {
                    vTaskDelay(5);
                    stop();
                }
                xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
                mutex_exit(&wifiMutex);
            }
        }

        if (mode == 's')
        {
            vTaskDelay(100); // wheels to stop completely
            target_code = g_left_wheel_code;
            printf("lc was %llu, rc was %llu, set tc to %d\n", g_left_wheel_code, g_right_wheel_code, target_code);
            long long right_offset = g_right_wheel_code - g_left_wheel_code;
            reset_wheel_encoder();
            g_right_wheel_code = right_offset;
            mode = 'r';
        }

        if (mode == 'r')
        {
            dist_last_error = dist_error;
            dist_error = target_code - g_left_wheel_code;
            derivative = dist_error - dist_last_error;
            // Code will increase going backwards too
            if (dist_error < 2)
            {
                control = 0;
                if (--steadycount == 0)
                {
                    mode = 'p';
                    steadycount = 50;
                }
            }
            else
            {
                control = fkp * dist_error;
                steadycount = 50;
            }
            control += fkd * derivative;
            if (control > 1)
                control = 1;
            set_speed(control * DEFAULT_SPEED);
            if (g_left_wheel_code < g_right_wheel_code)
            {
                right_tilt();
            }
            if (g_left_wheel_code > g_right_wheel_code)
            {
                left_tilt();
            }
            backwards();

            if (--update == 0)
            {
                update = 100;
                char update_data[100] = "";
                uint16_t speed = control * DEFAULT_SPEED;
                snprintf(update_data, 100, "[RVE]lc:%llu\tlr:%llu\ttar:%llu\terr:%d\tctrl:%.2f\tp:%.3f\td:%.2f\tspeed:%d\n", g_left_wheel_code, g_right_wheel_code, target_code, dist_error, control, fkp, derivative, speed);
                while (!mutex_try_enter(&wifiMutex, 0))
                {
                    vTaskDelay(5);
                    stop();
                }
                xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
                mutex_exit(&wifiMutex);
            }
        }

        if (mode == 't')
        {
            if (xMessageBufferReceive(h_turn_buffer, (void *)&read_bearing, sizeof(read_bearing), 0))
            {
                printf("readbearing: %d\n", read_bearing);
                if (read_bearing == 0)
                    target_bearing = current_bearing;
                target_bearing += read_bearing;
                if (target_bearing > 360)
                    target_bearing -= 360;
                if (target_bearing < 0)
                    target_bearing += 360;
            }
            bearing_last_error = bearing_error;
            intergral += bearing_error;
            derivative = bearing_error - bearing_last_error;

            if (abs(bearing_error) > 3)
            {
                control = tkp * bearing_error;
                steadycount = 50;
            }
            else
            {
                if (--steadycount == 0)
                {
                    mode = 'p';
                    steadycount = 50;
                }
                control = 0;
            }
            control += tki * intergral + tkd * derivative;

            if (control > 0)
            {
                // set direction here
                rotate_clockwise();
            }
            else
            {
                rotate_counter_clockwise();
            }
            if (control < 0)
                control = -control;
            if (control > 1)
                control = 1;
            set_speed(control * DEFAULT_SPEED);
            if (--update == 0)
            {
                update = 100;
                char update_data[100] = "";
                uint16_t speed = control * DEFAULT_SPEED;
                snprintf(update_data, 100, "[TUN]cur:%d\ttar:%d\terr:%d\tctrl:%f\tp:%.3f\tspeed:%d\n", current_bearing, target_bearing, bearing_error, control, tkp, speed);
                while (!mutex_try_enter(&wifiMutex, 0))
                {
                    printf("waiting for mutex\n");
                    vTaskDelay(5);
                    stop();
                }
                xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
                mutex_exit(&wifiMutex);
            }
        }
        vTaskDelay(10);
    }
}

// task for calibrating
void calibrate_task()
{
    char calibuffer[100] = "run";

    int update = 200;
    int16_t x, y, z;
    while (1)
    {
        if (strncmp(calibuffer, "run", 3) != 0)
            vTaskDelay(5000);
        read_mag(&x, &y, &z);
        m_min.x = MIN(m_min.x, x);
        m_min.y = MIN(m_min.y, y);
        m_min.z = MIN(m_min.z, z);

        m_max.x = MAX(m_max.x, x);
        m_max.y = MAX(m_max.y, y);
        m_max.z = MAX(m_max.z, z);

        if (--update == 0)
        {
            update = 200;
            char update_data[100] = "";
            snprintf(update_data, 100, "[CAL]min: x:%d\ty%d\tz%d[CAL]max: x:%d\ty:%d\tz:%d\n", m_min.x, m_min.y, m_min.z, m_max.x, m_max.y, m_max.z);
            while (!mutex_try_enter(&wifiMutex, 0))
            {
                vTaskDelay(5);
            }
            xMessageBufferSend(wifiMsgBuffer, update_data, 100, 0);
            mutex_exit(&wifiMutex);
        }
        vTaskDelay(10);
    }
}

void vLaunch(void)
{

    h_move_mode_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    h_turn_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);
    h_dist_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    TaskHandle_t server_sampleRecv;    // Create a task handle for the server task.
    TaskHandle_t server_sampleRecvISR; // Create a task handle for the server task.
    TaskHandle_t movement_task;        // Create a task handle for the server task.
    TaskHandle_t sensor_task;          // Create a task handle for the server task.
    wifiMsgBuffer = xMessageBufferCreate(256);
    wifiMsgBufferFromISR = xMessageBufferCreate(256);

    printf("creating tasks\n");
    xTaskCreate(move_task, "TurningTask", configMINIMAL_STACK_SIZE * 4, NULL, 2, &movement_task);                                    // Create the server task.
    xTaskCreate(sense_task, "SensorTask", configMINIMAL_STACK_SIZE, NULL, 3, &sensor_task);                                          // Create the server task.
    xTaskCreate(server_forward_task, "ServerForwardTask", configMINIMAL_STACK_SIZE * 2, NULL, 1, &server_sampleRecv);                // Create the server task.
    xTaskCreate(server_forward_task_from_ISR, "ServerForwardTaskISR", configMINIMAL_STACK_SIZE * 2, NULL, 1, &server_sampleRecvISR); // Create the server task.
    printf("starting tasks\n");
    vTaskStartScheduler();
    printf("task scheduler failed to hold");
}

int main()
{                     // Main function of the program.
    stdio_init_all(); // Initialize standard I/O.

    initWifi();
    start_server(NULL);

    gpio_init(IR_LEFT_PIN);
    gpio_init(IR_RIGHT_PIN);
    adc_init();
    setup_ultrasonic_pins(TRI_PIN, ECHO_PIN);
    init_engine();
    gpio_init(left_wheel_encoder_pin);
    gpio_init(right_wheel_encoder_pin);

    // Get the slice num and initialise the motor
    init_motor(DEFAULT_SPEED);

    initializeI2C(); // Initialize I2C communication.
    initalize_acc(); // Configure the accelerometer.
    initalize_mag(); // Configure the magnetometer.

    gpio_set_irq_callback(&mainIRQhandler);
    gpio_set_irq_enabled(left_wheel_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(right_wheel_encoder_pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ADC_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(IR_LEFT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(IR_RIGHT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    vLaunch();
    return 0; // Return 0 to indicate successful program execution.
}