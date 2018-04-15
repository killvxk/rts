#include "error/rts_eh.h"
#include "socks/rts_sock_os.h"
#include "socks/rts_sock_buffer.h"
#include <stdio.h>
#include <string.h>

int main()
{	
	rts_eh_t t = rts_eh_create_generic();
	rts_eh_t* log = &t;	

	rts_sock_os_t os = rts_sock_os_create(log);
	
	os.global_start(log);

	rts_sock_t sock = os.open(log);

	rts_info(log, "Socket %d", sock._value);
	
	os.bind(log, sock, 3272);

	os.listen(log, sock);
	
	rts_sock_t incoming;

	if (os.accept(log, sock, &incoming)) {

		rts_sock_buffer_t buf = rts_sock_buffer_create(24, &os, incoming);
		
		bool disconnect = false;

		if (rts_sock_buffer_recv_until_full(&buf, log, true, &disconnect)) {

			if (disconnect) {
				rts_info(log, "Client disconnected");
			} else {
				rts_info(log, "%s", buf._data);
			}
		}

		os.close(log, incoming);
		rts_sock_buffer_destroy(&buf);
	}

	os.close(log, sock);

	os.global_stop(log);

	return 0;
}
