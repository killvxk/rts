#pragma once 
#include <stdbool.h>
#include "rts_sock.h"

typedef void rts_sock_set_destroy_handler(void* os_data);

typedef void rts_sock_set_add_handler(void* os_data, rts_sock_t sock);
typedef void rts_sock_set_clear_handler(void* os_data, rts_sock_t sock);
typedef bool rts_sock_set_is_set_handler(void* os_data, rts_sock_t sock);

// Set of sockets - opaque wrapper around fd_set et al.
typedef struct {

	void* os_specific;

	rts_sock_set_destroy_handler* destroy;

	// FD_SET
	rts_sock_set_add_handler* add;

	// FD_CLR
	rts_sock_set_clear_handler* clear;

	// FD_ISSET
	rts_sock_set_is_set_handler* is_set;

} rts_sock_set_t;

rts_sock_set_t* rts_sock_create_set();

void rts_sock_set_add(rts_sock_set_t* set, rts_sock_t sock);

void rts_sock_set_clear(rts_sock_set_t* set, rts_sock_t sock);

bool rts_sock_set_is_set(rts_sock_set_t* set, rts_sock_t sock);

void rts_sock_destroy_set(rts_sock_set_t* set);
