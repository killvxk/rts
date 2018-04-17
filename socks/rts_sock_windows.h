#pragma once
#include "rts_os.h"

#ifdef RTS_PLAT_WINDOWS

#include "rts_sock_os.h"
#include "rts_eh_os.h"
#include <winsock2.h>
#include <ws2tcpip.h>

// Warning: sock_set depends on this struct being plain and not containing ptrs
// for safe memcpy
typedef struct {

	fd_set set;

} rts_sock_windows_sock_set_t;

// Attach all Windows related handlers to a given OS socket layer
void rts_sock_windows_attach(rts_sock_os_t* os);

#endif