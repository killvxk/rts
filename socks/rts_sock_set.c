#include "rts_sock_set.h"

#include <stdlib.h>
#include <string.h>

rts_sock_set_t* rts_sock_create_set() {
	rts_sock_set_t* set = (rts_sock_set_t*)malloc(sizeof(rts_sock_set_t));
	set->os_specific = NULL;
	set->destroy = NULL;
	return set;
}

void rts_sock_destroy_set(rts_sock_set_t* set) {
	if (set->destroy != NULL) {
		set->destroy(set->os_specific);
	}

	set->destroy = NULL;
	set->os_specific = NULL;
	free(set);
}

void rts_sock_set_add(rts_sock_set_t* set, rts_sock_t sock) {
	set->add(set->os_specific, sock);
}

void rts_sock_set_clear(rts_sock_set_t* set, rts_sock_t sock) {
	set->clear(set->os_specific, sock);
}

bool rts_sock_set_is_set(rts_sock_set_t* set, rts_sock_t sock) {
	set->is_set(set->os_specific, sock);
}