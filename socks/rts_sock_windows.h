#pragma once
#include "rts_os.h"

#ifdef RTS_PLAT_WINDOWS

#include "rts_sock_os.h"
#include "rts_eh_os.h"
#include <winsock.h>

// Attach all Windows related handlers to a given OS socket layer
void rts_sock_windows_attach(rts_sock_os_t* sock);

#endif