#pragma once
#include "rts_sock_set.h"
#include "rts_sock_os.h"
#include "rts_expander.h"

// Higher level wrapper around a socket set - does not directly manipulate or
// use FD_SET etc. and can be iterated through
typedef struct {

	// All sockets. Access this for iteration
	rts_expander_t* all_socks;

	// Master set of sockets without states
	rts_sock_set_t* master;

	// Temporary used only for select() calls (since it mutates the master
	// sock set otherwise)
	rts_sock_set_t* select_temporary;

	rts_sock_os_t* owning_os;

} rts_sock_roster_t;

rts_sock_roster_t* rts_sock_roster_create(rts_sock_os_t* owning_os);

void rts_sock_roster_add(rts_sock_roster_t* r, rts_sock_t sock);

// Destroy a roster of sockets. Can optionally close all sockets
void rts_sock_roster_destroy(rts_sock_roster_t* r, bool close_all_socks);

void rts_sock_roster_remove(rts_sock_roster_t* r, int index);

bool rts_sock_roster_select(rts_eh_t* eh, rts_sock_roster_t* r);

bool rts_sock_roster_is_set(rts_sock_roster_t* r, rts_sock_t sock);