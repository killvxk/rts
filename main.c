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

	rts_sock_t listener = os.open(log);

	rts_info(log, "Socket %d", listener.value);
	
	os.bind(log, listener, 3272);

	os.listen(log, listener);

	rts_sock_set_t* set = os.create_sock_set();
	rts_sock_set_add(set, listener);

	while (1) {
		os.select(log, set, NULL);

		if (rts_sock_set_is_set(set, listener)) {
			rts_info(log, "Listener is set!");

			rts_sock_t discard;
			
			if (os.accept(log, listener, &discard)) {
				os.close(log, discard);
			}
		}
	}
	
	/*rts_sock_t incoming;

	rts_sock_set_t* set = os.create_sock_set();

	if (os.accept(log, sock, &incoming)) {

		rts_sock_set_add(set, incoming);
				
		const char* hello = "Hello Mr Mac\r\n";
		rts_sock_buffer_t buf = rts_sock_buffer_create(14, &os, incoming);

		rts_sock_buffer_copy_from(hello, 14, &buf, true);
		
		while (rts_sock_buffer_send_until_finished(&buf, log));

		os.close(log, incoming);
		rts_sock_buffer_destroy(&buf);
	}*/

	os.close(log, listener);

	rts_sock_destroy_set(set);

	os.global_stop(log);

	return 0;
}
