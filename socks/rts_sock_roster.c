#include "rts_sock_roster.h"

#include "rts_alloc.h"

void destruct_socket(void* data, void* userdata) {
	((rts_sock_os_t*)userdata)->close(NULL, *((rts_sock_t*)data));
}

rts_sock_roster_t* rts_sock_roster_create(rts_sock_os_t* owning_os) {
	rts_sock_roster_t* r = rts_alloc(0, sizeof(rts_sock_roster_t));

	// Room for 4 socks by default
	// TODO: Maybe should pass down an incoming error handler?
	r->all_socks = rts_expander_create(NULL, sizeof(rts_sock_t) * 4);
		
	r->owning_os = owning_os;
	r->master = owning_os->create_sock_set();

	rts_expander_register_destructor(r->all_socks, sizeof(rts_sock_t), r->owning_os, true, &destruct_socket);

	r->select_readable_temporary = owning_os->create_sock_set();
	r->select_writeable_temporary = owning_os->create_sock_set();

	return r;
}

void rts_sock_roster_destroy(rts_sock_roster_t* r, bool close_all_socks) {

	rts_expander_destroy(NULL, r->all_socks, close_all_socks, r->owning_os);
	rts_sock_destroy_set(r->master);

	rts_sock_destroy_set(r->select_readable_temporary);
	rts_sock_destroy_set(r->select_writeable_temporary);

	r->owning_os = NULL;
	r->all_socks = NULL;

	rts_free(r);
}

void rts_sock_roster_add(rts_sock_roster_t* r, rts_sock_t sock) {
	rts_expander_add_item(NULL, r->all_socks, &sock, sizeof(rts_sock_t));
	rts_sock_set_add(r->master, sock);
}

void rts_sock_roster_remove(rts_sock_roster_t* r, int index) {

	rts_sock_t sock;
	rts_expander_get_item(NULL, r->all_socks, index, &sock, sizeof(rts_sock_t));

	rts_sock_set_clear(r->master, sock);
	rts_expander_remove_item(NULL, r->all_socks, index, sizeof(rts_sock_t));
}

bool rts_sock_roster_select(rts_eh_t* eh, rts_sock_roster_t* r) {

	// select mutates the FD set, so copy the master set of sockets -> temporary
	rts_sock_copy_set(r->master, r->select_readable_temporary);
	rts_sock_copy_set(r->master, r->select_writeable_temporary);
	
	return r->owning_os->select(eh, r->select_readable_temporary, r->select_writeable_temporary);
}

bool rts_sock_roster_is_receive_ready(rts_sock_roster_t* r, rts_sock_t sock) {

	// Always use the select temporary, not the master
	return rts_sock_set_is_set(r->select_readable_temporary, sock);
}

bool rts_sock_roster_is_send_ready(rts_sock_roster_t* r, rts_sock_t sock) {

	return rts_sock_set_is_set(r->select_writeable_temporary, sock);
}

rts_sock_t rts_roster_get_sock(rts_sock_roster_t* r, int index) {
	rts_sock_t sock;

	rts_expander_get_item(NULL, r->all_socks, index, &sock, sizeof(rts_sock_t));

	return sock;
}