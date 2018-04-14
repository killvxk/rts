#include "rts_sock_linux.h"

#ifdef RTS_PLAT_LINUX

rts_sock_t open_socket_generic(rts_eh_t* eh) {

	rts_sock_t sock;
	sock._value = socket(AF_INET, SOCK_STREAM, 0);

	return sock;
}

void close_socket_generic(rts_eh_t* eh, rts_sock_t sock) {
	
	if (close(sock._value) != 0) {
		rts_panic(eh, "Failed to close socket!");
	}
}

void rts_sock_linux_attach(rts_sock_os_t* sock) {
	sock->open = &open_socket_generic;
	sock->close = &close_socket_generic;
}

#endif