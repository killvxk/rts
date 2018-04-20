#include "error/rts_eh.h"
#include "socks/rts_sock_os.h"
#include "socks/rts_sock_buffer.h"
#include "memory/rts_expander.h"
#include "socks/rts_sock_roster.h"

#include <stdio.h>
#include <string.h>

int main()
{	
	int test = 42;

	rts_eh_t t = rts_eh_create_generic();
	rts_eh_t* log = &t;	

	rts_sock_os_t os = rts_sock_os_create(log);
	
	os.global_start(log);

	rts_sock_t listener = os.open(log);

	rts_info(log, "Socket %d", listener.value);
	
	os.bind(log, listener, 3272);

	os.listen(log, listener);

	rts_sock_roster_t* roster = rts_sock_roster_create(&os);
	rts_sock_roster_add(roster, listener);
		
	bool exit = false;

	while (!exit) {
				
		if (!rts_sock_roster_select(log, roster)) {
			rts_warning(log, "Select problem");
		}

		// Always check the listener (index 0) first before iterating the set
		//		
		if (rts_sock_roster_is_set(roster, listener)) {
			rts_info(log, "Listener is set!");			
			
			rts_sock_t new_socket;

			if (os.accept(log, listener, &new_socket)) {
				rts_info(log, "New client!");
				rts_sock_roster_add(roster, new_socket);
			} else {
				rts_warning(log, "Select failed!");
			}
		}
			

		for (int i = 1; i < roster->all_socks->items; i++) {

			rts_sock_t current; 

			rts_expander_get_item(log, roster->all_socks, i, &current, sizeof(rts_sock_t));

			if (rts_sock_roster_is_set(roster, current)) {
				rts_info(log, "Socket %d has data", current.value);

				int bytes = 0;

				rts_sock_buffer_t buf = rts_sock_buffer_create(64, &os, current);

				bool ok = rts_sock_buffer_recv(&buf, log, &bytes);

				if (!ok || bytes == 0) {
					rts_info(log, "%d disconnected or errored", current.value);
					rts_sock_buffer_destroy(&buf);
					rts_sock_roster_remove(roster, i);
					break;
				}				
				
				rts_sock_buffer_null_terminate(&buf);
				rts_info(log, buf.data);

				if (buf.data[0] == 'q') {
					exit = true;
				}

				// Send to all other clients
				for (int other = 1; other < roster->all_socks->items; other++) {

					if (other == i) {
						continue;
					}

					int sent;

					rts_sock_t peer;

					rts_expander_get_item(log, roster->all_socks, other, &peer, sizeof(rts_sock_t));

					os.send(log, peer, buf.data, buf.length, &sent);
				}

				rts_sock_buffer_destroy(&buf);
			}
		}
	}

	rts_sock_roster_destroy(roster, true);
	
	os.global_stop(log);

	return 0;
}

