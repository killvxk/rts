#pragma once
#include <stdbool.h>
#include "rts_sock.h"
#include "rts_eh.h"

typedef rts_sock_t rts_sock_os_open_handler(rts_eh_t* eh);

typedef void rts_sock_os_close_handler(rts_eh_t* eh, rts_sock_t sock);


typedef bool rts_sock_os_start_handler(rts_eh_t* eh);

typedef void rts_sock_os_stop_handler(rts_eh_t* eh);


typedef struct {

	// Open an actual socket. *Always* TCP/IP 
	rts_sock_os_open_handler* open;

	// Close an actual socket
	rts_sock_os_close_handler* close;


	// Start OS-level socket support
	rts_sock_os_start_handler* global_start;

	// Stop OS-level socket support
	rts_sock_os_stop_handler* global_stop;

} rts_sock_os_t;

// Create an OS-specific socket helper
rts_sock_os_t rts_sock_os_create(rts_eh_t* eh);

