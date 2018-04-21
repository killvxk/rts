#pragma once
#include "rts_circular.h"
#include "rts_sock.h"

// Combined send/receive queues for a socket
typedef struct {

	rts_sock_t sock;

	rts_circular_t* send;

	rts_circular_t* receive;

} rts_sock_send_receive_t;

rts_sock_send_receive_t* rts_sock_send_receive_create(rts_eh_t* eh, rts_sock_t sock);

void rts_sock_send_receive_destroy(rts_sock_send_receive_t* s);