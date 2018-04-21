#pragma once
#include "rts_sock_roster.h"
#include "rts_expander.h"

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

} rts_sock_io_control_t;

rts_sock_io_control_t* rts_sock_io_control_create(rts_eh_t* eh, rts_sock_os_t* owning_os);

void rts_sock_io_control_add(rts_eh_t* eh, rts_sock_io_control_t* i, rts_sock_t sock);

void rts_sock_io_control_remove(rts_sock_io_control_t* io, int index);

void rts_sock_io_control_destroy(rts_eh_t* eh, rts_sock_io_control_t* i);