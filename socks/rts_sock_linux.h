#pragma once 
#include "rts_os.h"

#ifdef RTS_PLAT_LINUX

#include "rts_sock_os.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// Attach all Linux related hooks to a socket OS handler
void rts_sock_linux_attach(rts_sock_os_t* sock);


#endif