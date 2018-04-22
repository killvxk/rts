#pragma once
#include "rts_sock_roster.h"
#include "rts_expander.h"

typedef enum {

	RTS_E_SOCK_OK, 
	RTS_E_SOCK_WOULD_BLOCK, 
	RTS_E_SOCK_DISCONNECT_NOW

} RTS_E_SOCK_RESULT;

// High level control of socket I/O: 
// - Roster of sockets with FD_SET info
// - List of sets of queues for each socket with 
// -- Receive queue
// -- Send queue
//
typedef struct {

	rts_sock_roster_t* roster;

	// Set of send+receive buffers for each socket
	rts_expander_t* send_receive;

	rts_sock_os_t* owning_os;

} rts_sock_io_control_t;

rts_sock_io_control_t* rts_sock_io_control_create(rts_eh_t* eh, rts_sock_os_t* owning_os);

void rts_sock_io_control_add(rts_eh_t* eh, rts_sock_io_control_t* i, rts_sock_t sock);

void rts_sock_io_control_remove(rts_sock_io_control_t* io, int index);

void rts_sock_io_control_destroy(rts_eh_t* eh, rts_sock_io_control_t* i);

RTS_E_SOCK_RESULT rts_sock_io_control_recv(rts_eh_t* eh, rts_sock_io_control_t* i, int index);