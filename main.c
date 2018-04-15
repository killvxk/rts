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

	rts_info(log, "Socket %d", sock.value);
	
	os.bind(log, sock, 3272);

	os.listen(log, sock);
	
	rts_sock_t incoming;

	if (os.accept(log, sock, &incoming)) {
				
		const char* hello = "Hello Mr Mac\r\n";
		rts_sock_buffer_t buf = rts_sock_buffer_create(14, &os, incoming);

		rts_sock_buffer_copy_from(hello, 14, &buf, true);
		
		while (rts_sock_buffer_send_until_finished(&buf, log));

		os.close(log, incoming);
		rts_sock_buffer_destroy(&buf);
	}

	os.close(log, sock);

	os.global_stop(log);

	return 0;
}
