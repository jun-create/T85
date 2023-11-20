#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#define TCP_PORT 4242
#define BUF_SIZE 2048

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

extern TCP_SERVER_T *tcp_server_init(void);
extern err_t tcp_server_send_data(struct pbuf *p, void *arg, struct tcp_pcb *tpcb);
extern err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
extern err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
extern bool tcp_server_open(void *arg);

#endif