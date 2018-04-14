#pragma once
#include <stdbool.h>
#include "rts_eh.h"

typedef bool rts_sock_os_start_handler(rts_eh_t* eh);

typedef void rts_sock_os_stop_handler(rts_eh_t* eh);

typedef struct {

	rts_sock_os_start_handler* os_start;

	rts_sock_os_stop_handler* os_stop;

} rts_sock_os_t;

// Create an OS-specific socket helper
rts_sock_os_t rts_sock_os_create(rts_eh_t* eh);