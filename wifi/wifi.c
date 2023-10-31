// Description: This file contains the code for the TCP server, majority sourced from the Pico SDK examples.
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

#define TCP_PORT 4242
#define BUF_SIZE 2048

// TCP Packet structure
typedef struct TCP_SERVER_T_
{
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    char buffer_sent[BUF_SIZE];
    uint8_t buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

// Main function to initialize the TCP server
static TCP_SERVER_T *tcp_server_init(void)
{
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state)
    { // Check if memory allocation failed.
        return NULL;
    }
    return state;
}

// Checks if the lwIP stack has encountered an error
static void tcp_server_err(void *arg, err_t err)
{
    if (err != ERR_ABRT)
    { // Check if the error is not an abort error.
        printf("Error code: %d\n", err);
    }
}

// Function to send data to the client
err_t tcp_server_send_data(struct pbuf *p, void *arg, struct tcp_pcb *tpcb)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    int len = p->tot_len;
    if (p->tot_len > BUF_SIZE)
        len = BUF_SIZE;
    // copy payload into buffer
    strncpy(state->buffer_sent, p->payload, len);
    cyw43_arch_lwip_check();
    // send the buffer
    err_t err = tcp_write(tpcb, state->buffer_sent, len, TCP_WRITE_FLAG_COPY); // Write data to the TCP connection
    err_t err2 = tcp_write(tpcb, "hi\n", 3, TCP_WRITE_FLAG_COPY);              // Write data to the TCP connection
    // Check if the write operation failed
    if (err != ERR_OK || err2 != ERR_OK)
    {
        printf("[wifi] Failed to write data!\n");
        return err;
    }
    else
    {
        printf("[wifi] Data sent successfully!\n");
        return ERR_OK;
    }
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    // Check if the received packet buffer is invalid
    if (!p)
    {
        printf("[wifi] Something went wrong!\n");
        return ERR_BUF;
    }
    cyw43_arch_lwip_check();
    // Check if the received data is empty
    if (strlen(p->payload) == 0)
    {
        // Free the packet buffer
        pbuf_free(p);
        return ERR_OK;
    }
    // Check if the total length of received data is greater than 0
    else if (p->tot_len > 0)
    {
        // Print the received data
        printf("[wifi][Buffer] value: %0.s\n[wifi][Buffer] len: %d\n", p->tot_len, p->payload, p->tot_len);
    }
    // Send a response to the client
    tcp_server_send_data(p, arg, state->client_pcb);

    // Free packet buffer
    pbuf_free(p);
    return ERR_OK;
}

// Accept a client connection
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    // Check for errors or invalid client protocol control block
    if (err != ERR_OK || client_pcb == NULL)
    {
        printf("[wifi] Failure in accept\n");
        return ERR_VAL;
    }
    printf("[wifi] Client connected\n");
    state->client_pcb = client_pcb; // Store the client's protocol control block.
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);
    return ERR_OK;
}

// Open the server
static bool tcp_server_open(void *arg)
{
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    printf("[wifi] Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT); // Print a message with server details.
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    // Check if the PCB creation failed
    if (!pcb)
    {
        printf("[wifi] Failed to create pcb\n");
        return false;
    }
    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    // Check if the binding operation failed
    if (err)
    {
        printf("Failed to bind to port %u\n", TCP_PORT);
        return false;
    }
    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    // Check if listening failed
    if (!state->server_pcb)
    {
        printf("[wifi] Failed to listen\n");
        if (pcb)
        {
            tcp_close(pcb);
        }
        return false;
    }
    tcp_arg(state->server_pcb, state);                // Set the argument for the server PCB
    tcp_accept(state->server_pcb, tcp_server_accept); // Set the callback for accepting client connections
    return true;
}

// Start the server
void start_server(__unused void *params)
{
    // Initialize special hardware component
    if (cyw43_arch_init())
    {
        printf("[wifi] Failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("[wifi] Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("[wifi] Failed to connect.\n");
        return;
    }
    else
    {
        printf("[wifi] Connected.\n");
    }
    TCP_SERVER_T *state = tcp_server_init();
    // Check state of tcp server
    if (!state)
    {
        printf("[wifi] Failed to allocate state\n");
        return;
    }
    // Check if the server failed to start
    if (!tcp_server_open(state))
    {
        printf("[wifi] Failed to start server\n");
        return;
    }
    while (true)
    {
        vTaskDelay(1000);
    }
}

int main()
{
    stdio_init_all();
    stdio_usb_init();
    sleep_ms(5000);
    // Create the server task
    TaskHandle_t wifi_task;
    xTaskCreate(start_server, "StartServer", configMINIMAL_STACK_SIZE, NULL, 4, &wifi_task);
    // Other tasks may go here...

    // Start the FreeRTOS task scheduler
    vTaskStartScheduler();
    return 0;
}