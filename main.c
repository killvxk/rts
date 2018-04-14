#include "error/rts_eh.h"
#include "socks/rts_sock_os.h"
#include <stdio.h>

int main()
{		
	rts_eh_t t = rts_eh_create_generic();
	rts_eh_t* log = &t;

	rts_sock_os_t sock = rts_sock_os_create(log);

	sock.os_start(log);
	sock.os_stop(log);

	return 0;
}
