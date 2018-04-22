#include "error/rts_eh.h"
#include "socks/rts_sock_os.h"
#include "memory/rts_expander.h"
#include "socks/rts_sock_roster.h"
#include "memory/rts_circular.h"
#include "socks/rts_sock_io_control.h"

#include "common/rts_threading.h"
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
	
	rts_sock_io_control_t* io = rts_sock_io_control_create(log, &os); 
	rts_sock_io_control_add(log, io, listener);
		
	bool exit = false;
	
	while (!exit) {
				
		if (!rts_sock_roster_select(log, io->roster)) {
			rts_warning(log, "Select problem");
		}

		// Always check the listener (index 0) first before iterating the set
		//		
		if (rts_sock_roster_is_receive_ready(io->roster, listener)) {
			rts_info(log, "Listener is set!");			
			
			rts_sock_t new_socket;

			if (os.accept(log, listener, &new_socket)) {
				rts_info(log, "New client!");
				rts_sock_io_control_add(log, io, new_socket);
			} else {
				rts_warning(log, "Select failed!");
			}
		}			

		for (int i = 1; i < io->roster->all_socks->items; i++) {

			rts_sock_t current = rts_roster_get_sock(io->roster, i);

			if (rts_sock_roster_is_receive_ready(io->roster, current)) {

				RTS_E_SOCK_RESULT result = rts_sock_io_control_recv(log, io, i);

				if (result == RTS_E_SOCK_OK) {


				} else {

					// Ignore WOULD_BLOCK and come back later

					if (result == RTS_E_SOCK_DISCONNECT_NOW) {
						rts_sock_io_control_remove(io, i);
						break;
					}					
				}
			}

			if (rts_sock_roster_is_send_ready(io->roster, current)) {

			}
		}
	}

	rts_sock_io_control_destroy(log, io);
	
	os.global_stop(log);

	return 0;
}

