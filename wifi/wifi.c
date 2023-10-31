/*
 * This file is cutdown version of the Pico LWIP examples.
 * It is intended to check if the buffer is received and sent correctly.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/gpio.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ultrasonic/ultrasonic.h"
#include "irline/irline.h"

#define TCP_PORT 4242
#define BUF_SIZE 2048
#define WIFI_NAME "MJ_MkIV"
#define WIFI_PASS "NekoGirls"

typedef struct TCP_SERVER_T_
{
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    char buffer_sent[BUF_SIZE]; // buffer change data type
    char buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

static TCP_SERVER_T *tcp_server_init(void)
{
    // Allocate memory to state
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));

    // Return null if state failed to allocate memory
    if (!state)
    {
        return NULL;
    }

    return state;
}

static void tcp_server_err(void *arg, err_t err)
{
    if (err != ERR_ABRT)
    {
        // If not aborted, print error message
        printf("Error code: %d\n", err);
    }
}
err_t tcp_server_send_sensor_data(void *arg, char *data)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;

    // call sensor data function
    init_barcode();
    // Set buffer to sensor data
    for (;;)
    {
        // strcpy(state->buffer_sent, (char *)(ir_digital_value));
        strcpy(state->buffer_sent, ir_digital_value ? "HIGH" : "LOW");

        cyw43_arch_lwip_check();
        err_t err = tcp_write(state->client_pcb, , BUF_SIZE, TCP_WRITE_FLAG_COPY);

        if (err != ERR_OK)
        {
            printf("Failed to write data!\n");
            return err;
        }
        else
        {
            printf("Data sent successfully!\n");
            return ERR_OK;
        }
    }
}

err_t tcp_server_send_data(struct pbuf *p, void *arg, struct tcp_pcb *tpcb)
{                                              // Send data over the TCP connection.
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg; // Retrieve the server state from the argument.
    int len = p->tot_len;
    if (p->tot_len > BUF_SIZE)
        len = BUF_SIZE;
    strncpy(state->buffer_sent, p->payload, len);
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, state->buffer_sent, len, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {                                      // Check if the write operation failed.
        printf("Failed to write data!\n"); // Print an error message.
        return err;
    }
    else
    {
        printf("Data sent successfully!\n"); // Print a success message.
        return ERR_OK;
    }
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    // If value is NULL or an error occurs, print and return error
    if (!p)
    {
        printf("Error!\n");
        return ERR_BUF;
    }
    // create variable for printing and sending back
    char payload[BUF_SIZE];
    strcpy(payload, p->payload);

    cyw43_arch_lwip_check();
    // Prevents empty payload and double send
    if (strlen(p->payload) == 0)
    {
        pbuf_free(p);
        return ERR_OK;
    }
    // Print buffer value
    else if (p->tot_len > 0)
    {
        printf("Buffer value: %s\n", payload);
        printf("Buffer value: %s\n", p->payload);
    }

    // Free the received buffer
    pbuf_free(p);
    free(payload);

    char *print_help = "help:\ns, sen, sensor for sensor data\n enter or anything for send receive ok\n";
    // if else and call another function to send return message
    if ((strcmp(payload, "help") == 0) | (strcmp(payload, "help\n") == 0) | (strcmp(payload, "h") == 0))
    {
        tcp_write(tpcb, print_help, strlen(print_help), TCP_WRITE_FLAG_COPY);
    }
    else if ((strcmp(payload, "s") == 0) | (strcmp(payload, "sen") == 0) | (strcmp(payload, "sensor") == 0))
    {
        tcp_server_send_sensor_data(arg, "sensor data");
        // TODO
    }
    else
    {
        tcp_server_send_data(arg, state->client_pcb);
    }

    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;

    if (err != ERR_OK || client_pcb == NULL)
    {
        // If have error or no PCB, close connection
        printf("Failure in accept\n");

        return ERR_VAL;
    }

    printf("Client connected\n");

    // Set callback functions
    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    // Create PCB
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb)
    {
        printf("Failed to create pcb\n");

        return false;
    }

    // Bind to port
    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err)
    {
        printf("Failed to bind to port %u\n", TCP_PORT);

        return false;
    }

    // Set to listen
    state->server_pcb = tcp_listen_with_backlog(pcb, 1);

    if (!state->server_pcb)
    {
        // Close PCB if not listening
        printf("Failed to listen\n");

        if (pcb)
        {
            tcp_close(pcb);
        }

        return false;
    }

    // Set callback functions
    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

void start_server(__unused void *params)
{
    // Initialize cyw43
    if (cyw43_arch_init())
    {
        printf("Failed to initialise\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    // Connect to Wi-Fi
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_NAME, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect.\n");
        return;
    }
    else
    {
        printf("Connected.\n");
    }

    // Create state
    TCP_SERVER_T *state = tcp_server_init();

    if (!state)
    {
        printf("Failed to allocate state\n");
        return;
    }

    if (!tcp_server_open(state))
    {
        printf("Failed to start server\n");
        return;
    }

    while (true)
    {
        // Delay for tick(s)
        vTaskDelay(100);
    }
}

int main()
{
    stdio_init_all();
    stdio_usb_init();

    // Create tasks with different prio
    TaskHandle_t server_task;
    xTaskCreate(start_server, "StartServer", configMINIMAL_STACK_SIZE, NULL, 4, &server_task);

    // Start all tasks
    vTaskStartScheduler();

    return 0;
}