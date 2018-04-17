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

	rts_sock_set_t* select_only = os.create_sock_set();

	rts_sock_t discard;
	while (1) {

		rts_sock_copy_set(set, select_only);

		if (!os.select(log, select_only, NULL)) {
			rts_warning(log, "Select problem");
		}

		if (rts_sock_set_is_set(select_only, listener)) {
			rts_info(log, "Listener is set!");			
			
			if (os.accept(log, listener, &discard)) {				
				rts_info(log, "New client!");
				rts_sock_set_add(set, discard);
				break;
			} else {
				rts_warning(log, "Select failed!");
			}
		}
	}

	os.close(log, discard);
	os.close(log, listener);

	rts_sock_destroy_set(select_only);
	rts_sock_destroy_set(set);

	os.global_stop(log);

	return 0;
}
