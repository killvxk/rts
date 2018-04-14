#pragma once 
#include "rts_os.h"

#ifdef RTS_PLAT_LINUX

#include "rts_sock_os.h"
#include "rts_eh_os.h"

#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

// Attach all Linux related hooks to a socket OS handler
void rts_sock_linux_attach(rts_sock_os_t* sock);


#endif