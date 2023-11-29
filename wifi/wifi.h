#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>  // Include the standard input/output library for I/O operations.
#include <string.h>  // Include the string library for string operations.
#include <stdlib.h>  // Include the standard library for memory allocation and other functions.
#include "hardware/gpio.h"  // Include the Pico hardware GPIO library.

#include "pico/stdlib.h"  // Include the Pico standard library for Pico-specific functions.
#include "pico/cyw43_arch.h"  // Include a custom Pico library for a specific hardware component.

#include "lwip/ip4_addr.h"  // Include the lwIP library for IPv4 address handling.
#include "lwip/pbuf.h"  // Include the lwIP library for packet buffer management.
#include "lwip/tcp.h"  // Include the lwIP library for TCP communication.

#include "FreeRTOS.h"  // Include the FreeRTOS library for real-time operating system functionality.
#include "task.h"  // Include the FreeRTOS library for task management.
#include "message_buffer.h"

#define TCP_PORT 4242  // Define a constant for the TCP port number the server will use.
#define BUF_SIZE 2048  // Define a constant for the size of the data buffer.

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

typedef struct TCP_SERVER_T_ {  // Define a custom data structure for the TCP server.
    struct tcp_pcb *server_pcb;  // Pointer to the server's TCP protocol control block.
    struct tcp_pcb *client_pcb;  // Pointer to the client's TCP protocol control block.
    bool connected;  // Flag to indicate completion.
    char buffer_sent[BUF_SIZE];  // Buffer for sent data.
    uint8_t buffer_recv[BUF_SIZE];  // Buffer for received data.
    int sent_len;  // Length of sent data.
    int recv_len;  // Length of received data.
} TCP_SERVER_T;

static TCP_SERVER_T* tcp_server_init(void);
static void tcp_server_err(void *arg, err_t err);
err_t tcp_server_send_data(struct pbuf *p, TCP_SERVER_T *state);
extern err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
static bool tcp_server_open(void *arg);
void start_server(__unused void *params);
void ServerForwardTask();
void ServerForwardTaskFromISR();
void initWifi();

extern TCP_SERVER_T *myServer;
extern MessageBufferHandle_t wifiMsgBuffer;
extern MessageBufferHandle_t wifiMsgBufferFromISR;
#endif