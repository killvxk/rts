#pragma once
#include <stdbool.h>
#include "rts_sock.h"
#include "rts_eh.h"

typedef rts_sock_t rts_sock_os_open_handler(rts_eh_t* eh);

typedef void rts_sock_os_close_handler(rts_eh_t* eh, rts_sock_t sock);

typedef bool rts_sock_os_bind_handler(rts_eh_t* eh, rts_sock_t sock, int port);

typedef bool rts_sock_os_listen_handler(rts_eh_t* eh, rts_sock_t sock);

typedef bool rts_sock_os_start_handler(rts_eh_t* eh);

typedef void rts_sock_os_stop_handler(rts_eh_t* eh);


typedef struct {

	// Open an actual socket. *Always* TCP/IP 
	rts_sock_os_open_handler* open;

	// Close an actual socket
	rts_sock_os_close_handler* close;

	// Bind an existing socket. Always binds to a port on the current host
	rts_sock_os_bind_handler* bind;

	// Listen on a given socket. Implementations should auto-select sensible backlog values
	rts_sock_os_listen_handler* listen;

	// Start OS-level socket support
	rts_sock_os_start_handler* global_start;

	// Stop OS-level socket support
	rts_sock_os_stop_handler* global_stop;

} rts_sock_os_t;

// Create an OS-specific socket helper
rts_sock_os_t rts_sock_os_create(rts_eh_t* eh);

// Parse a port into a string, for use with the getaddrinfo service name member.
// CALLER MUST FREE THE RETURNED BUFFER
char* rts_sock_parse_port(int port);
