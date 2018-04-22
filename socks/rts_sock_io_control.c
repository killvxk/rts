#include "rts_sock_io_control.h"
#include "rts_alloc.h"
#include "rts_expander.h"
#include "rts_sock_send_receive.h"

void destruct_send_receive(void* data, void* userdata) {
	rts_sock_send_receive_destroy(*(rts_sock_send_receive_t**)data);
}

rts_sock_io_control_t* rts_sock_io_control_create(rts_eh_t* eh, rts_sock_os_t* owning_os) {
	rts_sock_io_control_t* i = rts_alloc(0, sizeof(rts_sock_io_control_t));
	i->roster = rts_sock_roster_create(owning_os);
	i->owning_os = owning_os;

	i->send_receive = rts_expander_create(eh, sizeof(rts_sock_send_receive_t*) * 4); // 4 socket spaces by default	
	rts_expander_register_destructor(i->send_receive, sizeof(rts_sock_send_receive_t*), NULL, true, &destruct_send_receive);

	return i;
}

void rts_sock_io_control_destroy(rts_eh_t* eh, rts_sock_io_control_t* i) {
	rts_sock_roster_destroy(i->roster, true);
	rts_expander_destroy(eh, i->send_receive, true, NULL);
	rts_free(i);
}

void rts_sock_io_control_add(rts_eh_t* eh, rts_sock_io_control_t* i, rts_sock_t sock) {

	rts_sock_roster_add(i->roster, sock);

	// Create send and receive buffers
	rts_sock_send_receive_t* sr = rts_sock_send_receive_create(eh, sock);

	rts_expander_add_item(eh, i->send_receive, &sr, sizeof(rts_sock_send_receive_t*));
}

void rts_sock_io_control_remove(rts_sock_io_control_t* io, int index) {
	rts_sock_roster_remove(io->roster, index);
	rts_expander_remove_item(NULL, io->send_receive, index, sizeof(rts_sock_send_receive_t*));
}

RTS_E_SOCK_RESULT rts_sock_io_control_recv(rts_eh_t* eh, rts_sock_io_control_t* i, int index) {
	
	rts_sock_send_receive_t* sr;

	rts_expander_get_item(eh, i->send_receive, index, &sr, sizeof(rts_sock_send_receive_t*));

	int bytes_read = 0;

	bool would_block = false;
	
	bool ok = i->owning_os->recv(eh, 
		sr->sock,
		sr->scratch, 
		sr->scratch_length,
		&bytes_read, 
		&would_block);

	if (!ok) {

		if (bytes_read == 0) {
			// Disconnect or error
			rts_info(eh, "Socket %d disconnected or errored", sr->sock.value);
			return RTS_E_SOCK_DISCONNECT_NOW;
		}

		if (would_block) {
			// Skip and come back
			return RTS_E_SOCK_WOULD_BLOCK;
		}

		// TODO: Anything else interpreted as immediate disconnect
		return RTS_E_SOCK_DISCONNECT_NOW;
	}

	// Write the incoming data to the socket's dedicated circular
	rts_circular_write(eh, sr->receive, sr->scratch, bytes_read);

	return RTS_E_SOCK_OK;
}