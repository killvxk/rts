#pragma once 
#include "rts_os.h"

#ifdef RTS_PLAT_LINUX

#include "rts_sock_os.h"
#include "rts_eh_os.h"

#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

// Warning: sock_set depends on this struct being plain and not containing ptrs
// for safe memcpy
typedef struct {

	fd_set set;

	// The value of the highest descriptor in the set, used to determine
	// nfds in select() calls
	int highest;

} rts_sock_linux_sock_set_t;

// Attach all Linux related hooks to a socket OS handler
void rts_sock_linux_attach(rts_sock_os_t* os);


#endif