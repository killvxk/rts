#pragma once
#include <stdbool.h>
#include "rts_sock.h"
#include "rts_eh.h"

typedef rts_sock_t rts_sock_os_open_handler(rts_eh_t* eh);

typedef void rts_sock_os_close_handler(rts_eh_t* eh, rts_sock_t sock);

typedef bool rts_sock_os_bind_handler(rts_eh_t* eh, rts_sock_t sock, int port);

typedef bool rts_sock_os_listen_handler(rts_eh_t* eh, rts_sock_t sock);

// TODO: Save off address info given by accept()
typedef bool rts_sock_os_accept_handler(rts_eh_t* eh, rts_sock_t listener, rts_sock_t* new_client);

typedef bool rts_sock_os_recv_handler(rts_eh_t* eh, rts_sock_t sock, char* buffer, int buffer_length, int* bytes_read);

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

	// Accept an incoming client on a given socket. 
	// New client socket is set to a -1 socket handle if this call fails
	rts_sock_os_accept_handler* accept;

	// Perform a receive for a given socket. Always blocking, always full receive - PEEK is implemented elsewhere.
	// bytes_read contains byte qty. OR 0 for disconnect
	rts_sock_os_recv_handler* recv;

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
