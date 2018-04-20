#pragma once
#include "rts_expander.h"

// Higher level wrapper around a socket set - does not directly manipulate or
// use FD_SET etc. and can be iterated through
typedef struct {

	// All sockets. Access this for iteration
	rts_expander_t* all_socks;

} rts_sock_roster_t;