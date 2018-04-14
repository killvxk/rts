#include "error/rts_eh.h"
#include "socks/rts_sock_os.h"
#include <stdio.h>

int main()
{	
	rts_eh_t t = rts_eh_create_generic();
	rts_eh_t* log = &t;	

	rts_sock_os_t os = rts_sock_os_create(log);
	
	os.global_start(log);

	rts_sock_t sock = os.open(log);

	rts_info(log, "Socket %d", sock._value);
	
	os.bind(log, sock, 3272);

	os.close(log, sock);

	os.global_stop(log);

	return 0;
}
