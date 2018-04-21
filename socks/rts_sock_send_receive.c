#include "rts_sock_send_receive.h"
#include "rts_alloc.h"

rts_sock_send_receive_t* rts_sock_send_receive_create(rts_eh_t* eh, rts_sock_t sock) {
	rts_sock_send_receive_t* s = rts_alloc(0, sizeof(rts_sock_send_receive_t));
	s->sock = sock;
	s->send = rts_circular_create(eh, 64);
	s->receive = rts_circular_create(eh, 64);
	return s;
}

void rts_sock_send_receive_destroy(rts_sock_send_receive_t* s) {
	rts_circular_destroy(NULL, s->receive);
	rts_circular_destroy(NULL, s->send);
	rts_free(s);
}